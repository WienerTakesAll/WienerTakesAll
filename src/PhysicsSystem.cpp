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
#include "PhysicsSystemUtils.h"
#include "VehicleWheelQueryResults.h"
#include "VehicleSceneQueryData.h"
#include "CarControlType.h"

PhysicsSystem::PhysicsSystem(AssetManager& asset_manager, PhysicsSettings& physics_settings)
    : g_foundation_(PxCreateFoundation(PX_FOUNDATION_VERSION, g_allocator_, g_error_callback_))
    , g_scale_()
    , g_pvd_(PxCreatePvd(*g_foundation_))
    , g_physics_(PxCreatePhysics(PX_PHYSICS_VERSION, *g_foundation_, g_scale_, false, g_pvd_))
    , g_cooking_(PxCreateCooking(PX_PHYSICS_VERSION, *g_foundation_, g_scale_))
    , g_scene_(NULL)
    , forward_drive_(0.0f)
    , horizontal_drive_(0.0f)
    , braking_force_(0.0f)
    , hand_break_(false)
    , num_vehicles_(0)
      // Allocate simulation data so we can switch from 3-wheeled to 4-wheeled cars by switching simulation data.
    , wheels_sim_data_4w_ (PxVehicleWheelsSimData::allocate(4))
      // Data to store reports for each wheel.
    , wheel_query_results (VehicleWheelQueryResults::allocate(MAX_NUM_4W_VEHICLES * 4))
      // Scene query data for to allow raycasts for all suspensions of all vehicles.
    , sq_data_ (VehicleSceneQueryData::allocate(MAX_NUM_4W_VEHICLES * 4))
    , sq_wheel_raycast_batch_query_(NULL)
    , asset_manager_(asset_manager)
    , settings_(physics_settings) {

    EventSystem::add_event_handler(EventType::ADD_CAR, &PhysicsSystem::handle_add_car, this);
    EventSystem::add_event_handler(EventType::ADD_TERRAIN_EVENT, &PhysicsSystem::handle_add_terrain, this);
    EventSystem::add_event_handler(EventType::CAR_CONTROL, &PhysicsSystem::handle_car_control, this);

    // Setup Visual Debugger
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    g_pvd_->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // Vehicle Physics Initialization
    PxInitVehicleSDK(*g_physics_);
    physx::PxVehicleSetBasisVectors(physx::PxVec3(0, 1, 0), physx::PxVec3(0, 0, 1));
    physx::PxVehicleSetUpdateMode(physx::PxVehicleUpdateMode::eVELOCITY_CHANGE);

    // Scene Initialization
    physx::PxSceneDesc scene_desc(g_physics_->getTolerancesScale());
    scene_desc.gravity = settings_.gravity;

    physx::PxDefaultCpuDispatcher* dispatcher = physx::PxDefaultCpuDispatcherCreate(3);
    scene_desc.cpuDispatcher = dispatcher;

#if PX_WINDOWS
    physx::PxCudaContextManagerDesc cudaContextManagerDesc;
    auto cuda_context_manager = PxCreateCudaContextManager(*g_foundation_, cudaContextManagerDesc);
    scene_desc.gpuDispatcher = cuda_context_manager->getGpuDispatcher();
#endif

    scene_desc.filterShader = &physx::PxDefaultSimulationFilterShader;
    assert(scene_desc.isValid());
    g_scene_ = g_physics_->createScene(scene_desc);

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
        physx::PxVehicleDrive4WRawInputData g_vehicle_input_data;
        g_vehicle_input_data.setDigitalAccel(true);
        g_vehicle_input_data.setAnalogAccel(std::max(forward_drive_, 0.f));
        g_vehicle_input_data.setAnalogBrake(braking_force_);
        g_vehicle_input_data.setAnalogHandbrake(hand_break_ * 1.0f);
        hand_break_ = false;
        g_vehicle_input_data.setAnalogSteer(horizontal_drive_);

        // smooth input data
        PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(
            settings_.g_pad_smoothing_data, // padSmoothing
            settings_.g_steer_vs_forward_speed_table, // steerVsForwardSpeedTable
            g_vehicle_input_data, // rawInputData
            0.16f / SIM_STEPS, // timestep
            false, // isVehicleInAir
            (PxVehicleDrive4W&)*vehicles_[0] // focusVehicle
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

        PxWheelQueryResult wheel_query_results[PX_MAX_NB_WHEELS];
        PxVehicleWheelQueryResult vehicle_query_results[] = {
            {
                wheel_query_results,
                vehicles_[0]->mWheelsSimData.getNbWheels()
            }
        };

        physx::PxVehicleUpdates(
            0.16f / SIM_STEPS,
            settings_.gravity,
            *surface_tire_pairs_,
            num_vehicles_,
            vehicles_,
            vehicle_query_results
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

void PhysicsSystem::handle_add_car(const Event& e) {
    PX_ASSERT(num_vehicles_ < MAX_NUM_4W_VEHICLES);

    int object_id = e.get_value<int>("object_id", true).first;
    physx::PxTransform transform(0.f, 0.f, 0.f);

    transform.p.x = e.get_value<int>("pos_x", true).first;
    transform.p.y = e.get_value<int>("pos_y", true).first;
    transform.p.z = e.get_value<int>("pos_z", true).first;
    MeshAsset* mesh = asset_manager_.get_mesh_asset(settings_.car_mesh_asset_path);

    // Construct new PhysicsComponent and add to list
    dynamic_objects_.emplace_back(object_id);
    PhysicsComponent<false>& vehicle = dynamic_objects_.back();
    vehicle.set_mesh(g_physics_, g_cooking_, mesh);

    auto vehicle_material = vehicle.get_material();
    auto vehicle_mesh = vehicle.get_mesh();

    float s = 0.2f;
    PxVec3 verts[] = {
        {s, s, s},
        { -s, s, s},
        { s, s, -s },
        { -s, s, -s },
        { s, -s, s },
        { -s, -s, s },
        { s, -s, -s },
        { -s, -s, -s }
    };

    PxConvexMesh* wheel_mesh[] = {
        create_wheel_convex_mesh(verts, 8, *g_physics_, *g_cooking_),
        create_wheel_convex_mesh(verts, 8, *g_physics_, *g_cooking_),
        create_wheel_convex_mesh(verts, 8, *g_physics_, *g_cooking_),
        create_wheel_convex_mesh(verts, 8, *g_physics_, *g_cooking_)
    };

    PxVec3 wheel_center_offsets[4];
    wheel_center_offsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT] = physx::PxVec3(-1.5f, 0.5f, 1.f);
    wheel_center_offsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] = physx::PxVec3(1.5f, 0.5f, 1.f);
    wheel_center_offsets[physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT] = physx::PxVec3(-1.5f, 0.5f, -1.f);
    wheel_center_offsets[physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT] = physx::PxVec3(1.5f, 0.5f, -1.f);

    create_4w_vehicle(
        *vehicle_material,
        settings_.car_mass,
        wheel_center_offsets,
        vehicle_mesh,
        wheel_mesh,
        transform,
        true
    );

    vehicle.set_actor(vehicles_[num_vehicles_ - 1]->getRigidDynamicActor());
    vehicle.set_transform(transform);
}

void PhysicsSystem::handle_add_terrain(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(settings_.arena_mesh);

    static_objects_.emplace_back(object_id);
    static_objects_.back().set_mesh(g_physics_, g_cooking_, mesh);

    surface_tire_pairs_ = create_friction_pairs(static_objects_.back().get_material());

    g_scene_->addActor(*static_objects_.back().get_actor());
}

void PhysicsSystem::handle_car_control(const Event& e) {
    switch (e.get_value<int>("type", true).first) {
        case CarControlType::FORWARD_DRIVE:
            forward_drive_ = e.get_value<float>("value", true).first;
            break;

        case CarControlType::BRAKE:
            braking_force_ = e.get_value<float>("value", true).first;
            break;

        case CarControlType::STEER:
            horizontal_drive_ = e.get_value<float>("value", true).first;
            break;

        case CarControlType::HAND_BRAKE:
            hand_break_ = e.get_value<float>("value", true).first;
            break;
    }
}

void PhysicsSystem::create_4w_vehicle (
    const PxMaterial& material,
    const PxF32 chassis_mass,
    const PxVec3* wheel_centre_offsets4,
    PxConvexMesh* chassis_convex_mesh,
    PxConvexMesh** wheel_convex_meshes4,
    const PxTransform& start_transform,
    const bool use_auto_gear_flag
) {
    PxVehicleWheelsSimData* wheels_sim_data = PxVehicleWheelsSimData::allocate(4);
    PxVehicleDriveSimData4W drive_sim_data;
    PxVehicleChassisData chassis_data;
    create_4w_vehicle_simulation_data(
        chassis_mass,
        chassis_convex_mesh,
        20.0f,
        wheel_convex_meshes4,
        wheel_centre_offsets4,
        *wheels_sim_data,
        drive_sim_data,
        chassis_data
    );

    // Instantiate and finalize the vehicle using physx.
    PxRigidDynamic* vehicle_actor =
        create_4w_vehicle_actor(
            chassis_data,
            wheel_convex_meshes4,
            chassis_convex_mesh,
            *g_scene_,
            *g_physics_,
            material
        );

    // Create a car.
    PxVehicleDrive4W* car = PxVehicleDrive4W::allocate(4);
    car->setup(g_physics_, vehicle_actor, *wheels_sim_data, drive_sim_data, 0);

    // Free the sim data because we don't need that any more.
    wheels_sim_data->free();

    // Don't forget to add the actor to the scenxe.
    {
        PxSceneWriteLock scoped_lock(*g_scene_);
        g_scene_->addActor(*vehicle_actor);
    }

    // Set up the mapping between wheel and actor shape.
    car->mWheelsSimData.setWheelShapeMapping(0, 0);
    car->mWheelsSimData.setWheelShapeMapping(1, 1);
    car->mWheelsSimData.setWheelShapeMapping(2, 2);
    car->mWheelsSimData.setWheelShapeMapping(3, 3);

    // Set up the scene query filter data for each suspension line.
    PxFilterData vehicle_qry_filter_data;
    vehicle_setup_vehicle_shape_query_filter_data(&vehicle_qry_filter_data);
    car->mWheelsSimData.setSceneQueryFilterData(0, vehicle_qry_filter_data);
    car->mWheelsSimData.setSceneQueryFilterData(1, vehicle_qry_filter_data);
    car->mWheelsSimData.setSceneQueryFilterData(2, vehicle_qry_filter_data);
    car->mWheelsSimData.setSceneQueryFilterData(3, vehicle_qry_filter_data);

    // Set the transform and the instantiated car and set it be to be at rest.
    // resetNWCar(startTransform, car);
    // Set the autogear mode of the instantiate car.
    car->mDriveDynData.setUseAutoGears(use_auto_gear_flag);

    // Increment the number of vehicles
    vehicles_[num_vehicles_] = car;
    vehicle_wheel_query_results_[num_vehicles_].nbWheelQueryResults = 4;
    vehicle_wheel_query_results_[num_vehicles_].wheelQueryResults = wheel_query_results->add_vehicle(4);
    num_vehicles_++;

    drive_sim_data_4w_ = drive_sim_data;
}

