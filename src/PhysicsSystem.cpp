#include <assert.h>
#include <stdio.h>

#include <SDL.h>
#include <glm/gtx/quaternion.hpp>
#include "vehicle/PxVehicleTireFriction.h"
#include "vehicle/PxVehicleWheels.h"

#include "PhysicsSystem.h"
#include "AssetManager.h"
#include "PhysicsSystem.h"
#include "PhysicsSettings.h"
#include "MeshStuff.h"
#include "PhysicsSystemUtils.h"
#include "VehicleWheelQueryResults.h"
#include "VehicleSceneQueryData.h"

using namespace physx;

PhysicsSystem::PhysicsSystem(AssetManager& asset_manager, PhysicsSettings& physics_settings)
    : g_foundation_(PxCreateFoundation(PX_FOUNDATION_VERSION, g_allocator_, g_error_callback_))
    , g_scale_()
    , g_pvd_(PxCreatePvd(*g_foundation_))
    , g_physics_(PxCreatePhysics(PX_PHYSICS_VERSION, *g_foundation_, g_scale_, false, g_pvd_))
    , g_cooking_(PxCreateCooking(PX_PHYSICS_VERSION, *g_foundation_, g_scale_))
    , g_scene_(NULL)
    , sq_wheel_raycast_batch_query_(NULL)
    , asset_manager_(asset_manager)
    , forward_drive_(0.0f)
    , horizontal_drive_(0.0f)
    , backward_drive_(0.0f)
    , hand_break_(false)
    , num_vehicles_(0)
      // Allocate simulation data so we can switch from 3-wheeled to 4-wheeled cars by switching simulation data.
    , wheels_sim_data_4w_ (PxVehicleWheelsSimData::allocate(4))
      // Scene query data for to allow raycasts for all suspensions of all vehicles.
    , sq_data_ (VehicleSceneQueryData::allocate(MAX_NUM_4W_VEHICLES * 4))
      // Data to store reports for each wheel.
    , wheel_query_results (VehicleWheelQueryResults::allocate(MAX_NUM_4W_VEHICLES * 4))
    , settings_(physics_settings) {

    EventSystem::add_event_handler(EventType::ADD_EXAMPLE_SHIP_EVENT, &PhysicsSystem::handle_add_example_ship, this);
    EventSystem::add_event_handler(EventType::ADD_TERRAIN_EVENT, &PhysicsSystem::handle_add_terrain, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &PhysicsSystem::handle_key_press, this);

    // Setup Visual Debugger
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    g_pvd_->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // Vehicle Physics Initialization
    PxInitVehicleSDK(*g_physics_);
    physx::PxVehicleSetBasisVectors(physx::PxVec3(0, 1, 0), physx::PxVec3(0, 0, 1));
    physx::PxVehicleSetUpdateMode(physx::PxVehicleUpdateMode::eVELOCITY_CHANGE);

    // Scene Initialization
    physx::PxSceneDesc sceneDesc(g_physics_->getTolerancesScale());
    sceneDesc.gravity = settings_.gravity;

    physx::PxDefaultCpuDispatcher* dispatcher = physx::PxDefaultCpuDispatcherCreate(3);
    sceneDesc.cpuDispatcher = dispatcher;

#if PX_WINDOWS
    physx::PxCudaContextManagerDesc cudaContextManagerDesc;
    auto mCudaContextManager = PxCreateCudaContextManager(*g_foundation_, cudaContextManagerDesc);
    sceneDesc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();
#endif

    sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
    assert(sceneDesc.isValid());
    g_scene_ = g_physics_->createScene(sceneDesc);

    // Initialise all vehicle ptrs to null.
    for (PxU32 i = 0; i < MAX_NUM_4W_VEHICLES; i++) {
        vehicles_[i] = NULL;
    }
}

PhysicsSystem::~PhysicsSystem() {
    physx::PxCloseVehicleSDK();

    g_cooking_->release();
    g_physics_->release();
    g_pvd_->release();
    g_foundation_->release();
}

void PhysicsSystem::update() {
    assert(g_scene_);

    const int SIM_STEPS = 4;

    for (int i = 0; i < SIM_STEPS; i++) {
        if (!num_vehicles_) {
            g_scene_->simulate(0.16f / SIM_STEPS);
            g_scene_->fetchResults(true);
            continue;
        }

        // Build vehicle input data
        physx::PxVehicleDrive4WRawInputData gVehicleInputData;
        gVehicleInputData.setDigitalAccel(true);
        gVehicleInputData.setAnalogAccel(std::max(forward_drive_, 0.f));
        gVehicleInputData.setAnalogBrake(backward_drive_);
        gVehicleInputData.setAnalogHandbrake(hand_break_ * 1.0f);
        hand_break_ = false;
        gVehicleInputData.setAnalogSteer(horizontal_drive_);

        PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(
            settings_.g_pad_smoothing_data,
            settings_.g_steer_vs_forward_speed_table,
            gVehicleInputData,
            0.16f / SIM_STEPS,
            false,
            (PxVehicleDrive4W&)*vehicles_[0]
        );

        if (NULL == sq_wheel_raycast_batch_query_) {
            sq_wheel_raycast_batch_query_ = sq_data_->setup_batched_scene_query(g_scene_);
        }

        PxVehicleSuspensionRaycasts(
            sq_wheel_raycast_batch_query_,
            num_vehicles_,
            vehicles_,
            sq_data_->get_raycast_query_result_buffer_size(),
            sq_data_->get_raycast_query_result_buffer()
        );

        PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];
        PxVehicleWheelQueryResult vehicleQueryResults[1] = {
            {
                wheelQueryResults,
                vehicles_[0]->mWheelsSimData.getNbWheels()
            }
        };

        physx::PxVehicleUpdates(
            0.16f / SIM_STEPS,
            settings_.gravity,
            *surface_tire_pairs_,
            1,
            vehicles_,
            vehicleQueryResults
        );

        g_scene_->simulate(0.16f / SIM_STEPS);
        g_scene_->fetchResults(true);
    }

    // Inform rest of system of new object locations
    for (auto& object : dynamic_objects_) {
        if (!object.is_valid()) {
            continue;
        }

        physx::PxTransform transform = object.get_actor()->getGlobalPose();

        EventSystem::queue_event(
            Event(
                EventType::OBJECT_TRANSFORM_EVENT,
                "object_id", static_cast<int>(object.get_id()),
                "pos_x", transform.p.x,
                "pos_y", transform.p.y,
                "pos_z", transform.p.z,
                "qua_w", transform.q.w,
                "qua_x", transform.q.x,
                "qua_y", transform.q.y,
                "qua_z", transform.q.z
            )
        );
    }
}

void PhysicsSystem::handle_add_example_ship(const Event& e) {
    if (num_vehicles_) {
        return;
    }

    int object_id = e.get_value<int>("object_id", true).first;
    physx::PxTransform transform(0.f, 0.f, 0.f);

    transform.p.x = e.get_value<int>("pos_x", true).first;
    transform.p.y = e.get_value<int>("pos_y", true).first;
    transform.p.z = e.get_value<int>("pos_z", true).first;
    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/carBoxModel.obj");

    dynamic_objects_.emplace_back(object_id);
    auto& vehicle = dynamic_objects_.back();
    vehicle.set_mesh(g_physics_, g_cooking_, mesh);
    auto mat = vehicle.get_material();
    auto mesh_mesh = vehicle.get_mesh();


    float s = 0.2f;
    PxVec3 verts[8] = {
        {s, s, s},
        { -s, s, s},
        { s, s, -s },
        { -s, s, -s },
        { s, -s, s },
        { -s, -s, s },
        { s, -s, -s },
        { -s, -s, -s }
    };

    PxConvexMesh* wheel_mesh[4] = {
        createWheelConvexMesh(verts, 8, *g_physics_, *g_cooking_),
        createWheelConvexMesh(verts, 8, *g_physics_, *g_cooking_),
        createWheelConvexMesh(verts, 8, *g_physics_, *g_cooking_),
        createWheelConvexMesh(verts, 8, *g_physics_, *g_cooking_)
    };

    PxVec3 wheelCenterOffsets[4];
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT] = physx::PxVec3(-1.5f, 0.5f, 1.f);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] = physx::PxVec3(1.5f, 0.5f, 1.f);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT] = physx::PxVec3(-1.5f, 0.5f, -1.f);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT] = physx::PxVec3(1.5f, 0.5f, -1.f);

    create_4w_vehicle(*g_scene_, *g_physics_, *g_cooking_, *mat, 1500.f, wheelCenterOffsets, mesh_mesh, wheel_mesh, transform, true);
    vehicle.set_actor(vehicles_[num_vehicles_ - 1]->getRigidDynamicActor());
    vehicle.set_transform(transform);
}

void PhysicsSystem::handle_add_terrain(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/Terrain.obj");

    static_objects_.emplace_back(object_id);
    static_objects_.back().set_mesh(g_physics_, g_cooking_, mesh);

    surface_tire_pairs_ = createFrictionPairs(static_objects_.back().get_material());

    g_scene_->addActor(*static_objects_.back().get_actor());
}

void PhysicsSystem::handle_key_press(const Event& e) {
    int player_id = e.get_value<int>("player_id", true).first;
    int key = e.get_value<int>("key", true).first;
    int value = e.get_value<int>("value", true).first;

    switch (key) {
        case SDLK_w:
            forward_drive_ = 0.5f;
            backward_drive_ = 0.0f;
            break;

        case SDLK_s:
            forward_drive_ = 0.0f;
            backward_drive_ = 0.5f;
            break;

        case SDLK_a:
            horizontal_drive_ = 0.5f;
            break;

        case SDLK_d:
            horizontal_drive_ = -0.5f;
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            forward_drive_ = (float)value / 32768;
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            backward_drive_ = (float)value / 32768;
            break;

        case SDL_CONTROLLER_AXIS_LEFTX:
            horizontal_drive_ = (float)value / -32768;
            break;

        case SDL_CONTROLLER_BUTTON_B:
            hand_break_ = true;

        default:
            break;
    }
}

void PhysicsSystem::create_4w_vehicle (
    PxScene& scene,
    PxPhysics& physics,
    PxCooking& cooking,
    const PxMaterial& material,
    const PxF32 chassisMass,
    const PxVec3* wheelCentreOffsets4,
    PxConvexMesh* chassisConvexMesh,
    PxConvexMesh** wheelConvexMeshes4,
    const PxTransform& startTransform,
    const bool useAutoGearFlag
) {
    PX_ASSERT(num_vehicles_ < MAX_NUM_4W_VEHICLES);

    PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(4);
    PxVehicleDriveSimData4W driveSimData;
    PxVehicleChassisData chassisData;
    createVehicle4WSimulationData
    (chassisMass, chassisConvexMesh,
     20.0f, wheelConvexMeshes4, wheelCentreOffsets4,
     *wheelsSimData, driveSimData, chassisData);

    // Instantiate and finalize the vehicle using physx.
    PxRigidDynamic* vehActor = createVehicleActor4W(chassisData, wheelConvexMeshes4, chassisConvexMesh, scene, physics, material);

    // Create a car.
    PxVehicleDrive4W* car = PxVehicleDrive4W::allocate(4);
    car->setup(&physics, vehActor, *wheelsSimData, driveSimData, 0);

    // Free the sim data because we don't need that any more.
    wheelsSimData->free();

    // Don't forget to add the actor to the scene.
    {
        PxSceneWriteLock scopedLock(scene);
        scene.addActor(*vehActor);
    }

    // Set up the mapping between wheel and actor shape.
    car->mWheelsSimData.setWheelShapeMapping(0, 0);
    car->mWheelsSimData.setWheelShapeMapping(1, 1);
    car->mWheelsSimData.setWheelShapeMapping(2, 2);
    car->mWheelsSimData.setWheelShapeMapping(3, 3);

    // Set up the scene query filter data for each suspension line.
    PxFilterData vehQryFilterData;
    SampleVehicleSetupVehicleShapeQueryFilterData(&vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(0, vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(1, vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(2, vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(3, vehQryFilterData);

    // Set the transform and the instantiated car and set it be to be at rest.
    // resetNWCar(startTransform, car);
    // Set the autogear mode of the instantiate car.
    car->mDriveDynData.setUseAutoGears(useAutoGearFlag);

    // Increment the number of vehicles
    vehicles_[num_vehicles_] = car;
    vehicle_wheel_query_results_[num_vehicles_].nbWheelQueryResults = 4;
    vehicle_wheel_query_results_[num_vehicles_].wheelQueryResults = wheel_query_results->add_vehicle(4);
    num_vehicles_++;

    drive_sim_data_4w_ = driveSimData;
}

