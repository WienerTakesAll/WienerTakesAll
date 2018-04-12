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
#include "GameState.h"

PhysicsSystem::PhysicsSystem(AssetManager& asset_manager, const PhysicsSettings& physics_settings)
    : g_foundation_(PxCreateFoundation(PX_FOUNDATION_VERSION, g_allocator_, g_error_callback_))
    , g_pvd_(PxCreatePvd(*g_foundation_))
    , g_physics_(PxCreatePhysics(PX_PHYSICS_VERSION, *g_foundation_, physx::PxTolerancesScale(), false, g_pvd_))
    , g_cooking_(PxCreateCooking(PX_PHYSICS_VERSION, *g_foundation_, physx::PxTolerancesScale()))
    , g_scene_(NULL)
    , vehicle_controls_(4) // 4 players
      // Scene query data for to allow raycasts for all suspensions of all vehicles.
    , sq_data_(VehicleSceneQueryData::allocate(MAX_NUM_4W_VEHICLES * 4))
    , asset_manager_(asset_manager)
    , settings_(physics_settings)
    , arena_material_(g_physics_->createMaterial(5.f, 5.f, 5.f))
    , friction_pair_service_(settings_.arena_tire_friction, arena_material_) {

    EventSystem::add_event_handler(EventType::ADD_VEHICLE, &PhysicsSystem::handle_add_vehicle, this);
    EventSystem::add_event_handler(EventType::ADD_ARENA, &PhysicsSystem::handle_add_arena, this);
    EventSystem::add_event_handler(EventType::ADD_CHARCOAL, &PhysicsSystem::handle_add_charcoal, this);
    EventSystem::add_event_handler(EventType::VEHICLE_CONTROL, &PhysicsSystem::handle_vehicle_control, this);
    EventSystem::add_event_handler(EventType::RELOAD_SETTINGS_EVENT, &PhysicsSystem::handle_reload_settings, this);
    EventSystem::add_event_handler(EventType::OBJECT_APPLY_FORCE, &PhysicsSystem::handle_object_apply_force, this);
    EventSystem::add_event_handler(EventType::NEW_GAME_STATE, &PhysicsSystem::handle_new_game_state, this);

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

    scene_desc.simulationEventCallback = &collision_events_subsystem_;
    scene_desc.filterShader = wienerTakesAllFilterShader;
    // scene_desc.filterShader = &physx::PxDefaultSimulationFilterShader;

#if PX_WINDOWS
    physx::PxCudaContextManagerDesc cudaContextManagerDesc;
    auto cuda_context_manager = PxCreateCudaContextManager(*g_foundation_, cudaContextManagerDesc);
    scene_desc.gpuDispatcher = cuda_context_manager->getGpuDispatcher();
#endif

    assert(scene_desc.isValid());
    g_scene_ = g_physics_->createScene(scene_desc);
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

    for (std::pair<int, int> collision : collision_events_subsystem_.consume_collisions()) {
        bool a_is_vehicle = false;
        bool b_is_vehicle = false;

        for (auto& dynamic_object : dynamic_objects_) {
            if (dynamic_object.get_id() == collision.first && dynamic_object.is_vehicle()) {
                a_is_vehicle = true;
            }

            if (dynamic_object.get_id() == collision.second && dynamic_object.is_vehicle()) {
                b_is_vehicle = true;
            }
        }

        if (a_is_vehicle && b_is_vehicle) {
            EventSystem::queue_event(
                Event(
                    EventType::VEHICLE_COLLISION,
                    "a_id", collision.first,
                    "b_id", collision.second
                )
            );
        }
    }

    const int SIM_STEPS = 4;
    const float TIME_PER_UPDATE = 0.16f;

    for (int i = 0; i < SIM_STEPS; i++) {
        std::vector<PxVehicleWheelQueryResult> vehicle_query_results;

        for (unsigned int i = 0; i < vehicles_.size(); i++) {
            // Build vehicle input data
            physx::PxVehicleDrive4WRawInputData g_vehicle_input_data;
            g_vehicle_input_data.setDigitalAccel(true);

            float accelForce = std::max(vehicle_controls_[i].forward_drive, vehicle_controls_[i].braking_force * 0.85f);
            g_vehicle_input_data.setAnalogAccel(std::max(accelForce, 0.f));

            g_vehicle_input_data.setAnalogHandbrake(vehicle_controls_[i].hand_break * 1.0f);
            vehicle_controls_[i].hand_break = false;
            g_vehicle_input_data.setAnalogSteer(vehicle_controls_[i].horizontal_drive);

            if (vehicle_controls_[i].braking_force * 0.65f < vehicle_controls_[i].forward_drive) {
                g_vehicle_input_data.setGearUp(1);
                vehicle_controls_[i].startedReversing = false;
            } else {
                g_vehicle_input_data.setGearDown(1);
                auto vel = vehicles_[i]->getRigidDynamicActor()->getLinearVelocity();

                if (vel.magnitude() < 1.f) {
                    vehicle_controls_[i].startedReversing = true;
                }
            }

            if (vehicle_controls_[i].startedReversing) {
                g_vehicle_input_data.setGearDown(1);
            } else {
                g_vehicle_input_data.setAnalogBrake(vehicle_controls_[i].braking_force);
            }




            // smooth input data
            PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(
                settings_.g_pad_smoothing_data, // padSmoothing
                settings_.g_steer_vs_forward_speed_table, // steerVsForwardSpeedTable
                g_vehicle_input_data, // rawInputData
                TIME_PER_UPDATE / SIM_STEPS, // timestep
                false, // isVehicleInAir
                (PxVehicleDrive4W&)*vehicles_[i] // focusVehicle
            );

            PxWheelQueryResult wheel_query_results[PX_MAX_NB_WHEELS];
            vehicle_query_results.push_back({
                wheel_query_results,
                vehicles_[i]->mWheelsSimData.getNbWheels()
            });
        }

        physx::PxBatchQuery* sq_wheel_raycast_batch_query = sq_data_->setup_batched_scene_query(g_scene_);

        if (vehicles_.size()) {
            PxVehicleSuspensionRaycasts(
                sq_wheel_raycast_batch_query,
                vehicles_.size(),
                &vehicles_[0],
                sq_data_->get_raycast_query_result_buffer_size(),
                sq_data_->get_raycast_query_result_buffer()
            );

            sq_wheel_raycast_batch_query->release();

            physx::PxVehicleUpdates(
                TIME_PER_UPDATE / SIM_STEPS,
                settings_.gravity,
                friction_pair_service_.get_friction_pairs(),
                vehicles_.size(),
                &vehicles_[0],
                &vehicle_query_results[0]
            );
        }



        g_scene_->simulate(TIME_PER_UPDATE / SIM_STEPS);
        g_scene_->fetchResults(true);
    }

    // Inform rest of system of new object locations
    for (auto& object : dynamic_objects_) {
        if (!object.is_valid()) {
            continue;
        }

        physx::PxTransform transform = object.get_actor()->getGlobalPose();

        //If the player is out of the arena, put them back in
        if (transform.p.y < -30) {
            transform.p.x = 0;
            transform.p.y = 20;
            transform.p.z = 0;
            transform.q.w = 1;
            transform.q.x = 0;
            transform.q.y = 0;
            transform.q.z = 0;
            object.get_actor()->setGlobalPose(transform);
            object.get_actor()->setLinearVelocity(PxVec3(0.f, 1.f, 0.f));
            object.get_actor()->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));

            EventSystem::queue_event(
                Event(
                    EventType::PLAYER_FELL_OFF_ARENA,
                    "object_id", static_cast<int>(object.get_id())
                )
            );
        }

        physx::PxVec3 speed = object.get_actor()->getLinearVelocity();

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
                "qua_z", transform.q.z,
                "vel_x", speed.x,
                "vel_y", speed.y,
                "vel_z", speed.z
            )
        );
    }
}

void PhysicsSystem::handle_add_vehicle(const Event& e) {
    PX_ASSERT(vehicles_.size() < MAX_NUM_4W_VEHICLES);

    int object_id = e.get_value<int>("object_id", true).first;
    physx::PxTransform transform(0.f, 0.f, 0.f);

    transform.p.x = e.get_value<int>("pos_x", true).first;
    transform.p.y = e.get_value<int>("pos_y", true).first;
    transform.p.z = e.get_value<int>("pos_z", true).first;
    float rotation = e.get_value<float>("rotation", true).first;
    glm::quat rotation_quat = glm::rotate(glm::quat(), glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    transform.q.w = rotation_quat.w;
    transform.q.x = rotation_quat.x;
    transform.q.y = rotation_quat.y;
    transform.q.z = rotation_quat.z;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(settings_.vehicle_mesh_asset_path);

    // Construct new PhysicsComponent and add to list
    dynamic_objects_.emplace_back(object_id);
    PhysicsComponent<false>& vehicle = dynamic_objects_.back();
    vehicle.set_mesh(g_physics_, g_cooking_, mesh);

    // construct wheel meshes
    float s = 0.2f;
    std::vector<PxVec3> verts = {
        {s, s, s},
        { -s, s, s},
        { s, s, -s },
        { -s, s, -s },
        { s, -s, s },
        { -s, -s, s },
        { s, -s, -s },
        { -s, -s, -s }
    };

    std::vector<PxConvexMesh*> wheel_meshes;

    for (int i = 0; i < 4; i++) { // 4 wheels in a car
        wheel_meshes.push_back(
            wheel_mesh_generator_.create_wheel_convex_mesh(
                &verts[0],
                verts.size(),
                *g_physics_,
                *g_cooking_
            )
        );
    }

    // create vehicle
    create_4w_vehicle(
        *vehicle.get_material(),
        vehicle.get_mesh(),
        &wheel_meshes[0]
    );

    vehicle.set_actor(vehicles_.back()->getRigidDynamicActor());
    vehicle.set_transform(transform);
    vehicle.set_is_vehicle(true);
    vehicle_controls_.push_back(VehicleControls());
}

void PhysicsSystem::handle_add_arena(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(settings_.arena_mesh);

    static_objects_.emplace_back(object_id);
    static_objects_.back().set_mesh(g_physics_, g_cooking_, mesh);
    static_objects_.back().set_material(arena_material_);

    g_scene_->addActor(*static_objects_.back().get_actor());
}

void PhysicsSystem::handle_add_charcoal(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    physx::PxTransform transform(0.f, 0.f, 0.f);

    transform.p.x = e.get_value<int>("pos_x", true).first;
    transform.p.y = e.get_value<int>("pos_y", true).first;
    transform.p.z = e.get_value<int>("pos_z", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(settings_.charcoal_mesh);

    static_objects_.emplace_back(object_id);
    static_objects_.back().set_mesh(g_physics_, g_cooking_, mesh, transform);
    static_objects_.back().set_material(arena_material_);

    g_scene_->addActor(*static_objects_.back().get_actor());
}



void PhysicsSystem::handle_vehicle_control(const Event& e) {
    int vehicle_index = e.get_value<int>("index", true).first;

    switch (e.get_value<int>("type", true).first) {
        case VehicleControlType::FORWARD_DRIVE:
            vehicle_controls_[vehicle_index].forward_drive = e.get_value<float>("value", true).first;
            break;

        case VehicleControlType::BRAKE:
            vehicle_controls_[vehicle_index].braking_force = e.get_value<float>("value", true).first;
            break;

        case VehicleControlType::STEER:
            vehicle_controls_[vehicle_index].horizontal_drive = e.get_value<float>("value", true).first;
            break;

        case VehicleControlType::HAND_BRAKE:
            vehicle_controls_[vehicle_index].hand_break = e.get_value<float>("value", true).first;
            break;
    }
}

void PhysicsSystem::handle_reload_settings(const Event& e) {
    std::cout << "Setting scene gravity" << std::endl;
    g_scene_->setGravity(settings_.gravity);

    std::cout << "Setting friction data" << std::endl;
    friction_pair_service_.set_friction_data(settings_.arena_tire_friction);
}

void PhysicsSystem::handle_object_apply_force(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    float x = e.get_value<float>("x", true).first;
    float y = e.get_value<float>("y", true).first;
    float z = e.get_value<float>("z", true).first;

    for (PhysicsComponent<false> dynamic_object : dynamic_objects_) {
        if (dynamic_object.get_id() == object_id) {
            dynamic_object.get_actor()->addForce(PxVec3(x, y, z));
            return;
        }
    }
}


void PhysicsSystem::handle_new_game_state(const Event& e) {
    GameState new_game_state = (GameState)e.get_value<int>("state", true).first;


    if (new_game_state == GameState::START_MENU) {
        for (auto& dynamic_object : dynamic_objects_) {
            dynamic_object.get_actor()->release();
        }

        for (auto& static_object : static_objects_) {
            static_object.get_actor()->release();
        }

        for (auto& wheels : vehicles_) {
            wheels->release();
        }

        for (auto& control : vehicle_controls_) {
            control.forward_drive = 0.f;
            control.braking_force = 0.f;
            control.hand_break = 0.f;
            control.horizontal_drive = 0.f;
        }

        dynamic_objects_.clear();
        static_objects_.clear();
        vehicles_.clear();
    }


    if (new_game_state == IN_GAME) {
        for (auto& object : dynamic_objects_) {
            object.get_actor()->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
            object.get_actor()->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));
        }

        for (auto& control : vehicle_controls_) {
            control = VehicleControls();
        }
    }

    if (new_game_state == GameState::END_GAME) {
        for (auto& control : vehicle_controls_) {
            control.forward_drive = 0.f;
            control.braking_force = 0.f;
            control.hand_break = 0.f;
            control.horizontal_drive = 0.f;
        }
    }

}

void PhysicsSystem::handle_set_chassis_mass(const Event& e) {
    std::pair<int, bool> object_id = e.get_value<int>("object_id", true);
    std::pair<float, bool> mass = e.get_value<float>("mass", true);
    vehicles_[object_id.first]->mWheelsSimData.setChassisMass(mass.first);
}

void PhysicsSystem::handle_restore_chassis_mass(const Event& e) {
    std::pair<int, bool> object_id = e.get_value<int>("object_id", true);
    vehicles_[object_id.first]->mWheelsSimData.setChassisMass(settings_.vehicle_mass);
}

void PhysicsSystem::create_4w_vehicle (
    const PxMaterial& material,
    PxConvexMesh* chassis_convex_mesh,
    PxConvexMesh** wheel_convex_meshes_4
) {
    PxVehicleChassisData chassis_data =
        create_chassis_data(
            settings_.vehicle_mass,
            chassis_convex_mesh
        );

    PxRigidDynamic* vehicle_actor =
        create_4w_vehicle_actor(
            chassis_data,
            wheel_convex_meshes_4,
            chassis_convex_mesh,
            *g_scene_,
            *g_physics_,
            material
        );

    PxVehicleDriveSimData4W drive_sim_data =
        create_drive_sim_data(
            settings_.wheel_center_offsets
        );

    PxVehicleWheelsSimData* wheels_sim_data =
        create_wheels_sim_data(
            chassis_data,
            settings_.wheel_mass,
            wheel_convex_meshes_4,
            settings_.wheel_center_offsets
        );

    // Create a vehicle.
    const int NUM_WHEELS = 4;
    PxVehicleDrive4W* vehicle = PxVehicleDrive4W::allocate(NUM_WHEELS);
    vehicle->setup(g_physics_, vehicle_actor, *wheels_sim_data, drive_sim_data, 0);

    // Free the sim data because we don't need that any more.
    wheels_sim_data->free();

    // Set up the mapping between wheel and actor shape.
    vehicle->mWheelsSimData.setWheelShapeMapping(0, 0);
    vehicle->mWheelsSimData.setWheelShapeMapping(1, 1);
    vehicle->mWheelsSimData.setWheelShapeMapping(2, 2);
    vehicle->mWheelsSimData.setWheelShapeMapping(3, 3);

    // Set up the scene query filter data for each suspension line.
    PxFilterData vehicle_qry_filter_data;
    vehicle_setup_vehicle_shape_query_filter_data(&vehicle_qry_filter_data);
    vehicle->mWheelsSimData.setSceneQueryFilterData(0, vehicle_qry_filter_data);
    vehicle->mWheelsSimData.setSceneQueryFilterData(1, vehicle_qry_filter_data);
    vehicle->mWheelsSimData.setSceneQueryFilterData(2, vehicle_qry_filter_data);
    vehicle->mWheelsSimData.setSceneQueryFilterData(3, vehicle_qry_filter_data);
    vehicle->mWheelsSimData.setChassisMass(settings_.vehicle_mass);

    // Set the autogear mode of the instantiate vehicle.
    vehicle->mDriveDynData.setUseAutoGears(true);

    // Don't forget to add the actor to the scene.
    {
        PxSceneWriteLock scoped_lock(*g_scene_);
        g_scene_->addActor(*vehicle_actor);
    }

    // Increment the number of vehicles
    vehicles_.push_back(vehicle);
}
