#include <vector>

#include "VehiclePhysicsSystem.h"

namespace {
    const float SIDE = 0.2f; //

    const std::vector<PxVec3> WHEEL_VERTS = {
        { SIDE, SIDE, SIDE },
        { -SIDE, SIDE, SIDE},
        { SIDE, SIDE, -SIDE },
        { -SIDE, SIDE, -SIDE },
        { SIDE, -SIDE, SIDE },
        { -SIDE, -SIDE, SIDE },
        { SIDE, -SIDE, -SIDE },
        { -SIDE, -SIDE, -SIDE }
    };

    const std::vector<PxVec3> WHEEL_CENTER_OFFSETS = {
        { -1.5f, 0.5f, 1.f }, // physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT
        { 1.5f, 0.5f, 1.f }, // physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT
        { -1.5f, 0.5f, -1.f }, // physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT
        { 1.5f, 0.5f, -1.f } // physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT
    };
}

VehiclePhysicsSystem::VehiclePhysicsSystem(
    physx::PxPhysics& physics,
    physx::PxCooking& cooking
)
    : physics_(physics)
    , cooking_(cooking) {

    for (int i = 0; i < 4; i++) { // 4 wheels in a car
        wheel_meshes_template.push_back(
            wheel_mesh_generator_.create_wheel_convex_mesh(
                &WHEEL_VERTS[0],
                WHEEL_VERTS.size(),
                physics_,
                cooking_
            )
        );
    }
}

VehiclePhysicsSystem::~VehiclePhysicsSystem() {}

void VehiclePhysicsSystem::update() {

}

PhysicsComponent<false> VehiclePhysicsSystem::create_4w_vehicle(
    int id,
    physx::PxTransform transform,
    MeshAsset* mesh
) {
    auto new_vehicle = PhysicsComponent<false>(id);
    new_vehicle.set_mesh(&physics_, &cooking_, mesh);

    // PxVehicleWheelsSimData* wheels_sim_data = PxVehicleWheelsSimData::allocate(4);
    // PxVehicleDriveSimData4W drive_sim_data;
    // PxVehicleChassisData chassis_data;
    // create_4w_vehicle_simulation_data(
    //     chassis_mass,
    //     chassis_convex_mesh,
    //     20.0f,
    //     wheel_convex_meshes4,
    //     wheel_centre_offsets4,
    //     *wheels_sim_data,
    //     drive_sim_data,
    //     chassis_data
    // );

    // // Instantiate and finalize the vehicle using physx.
    // PxRigidDynamic* vehicle_actor =
    //     create_4w_vehicle_actor(
    //         chassis_data,
    //         wheel_convex_meshes4,
    //         chassis_convex_mesh,
    //         *g_scene_,
    //         *g_physics_,
    //         material
    //     );

    // // Create a vehicle.
    // PxVehicleDrive4W* vehicle = PxVehicleDrive4W::allocate(4);
    // vehicle->setup(g_physics_, vehicle_actor, *wheels_sim_data, drive_sim_data, 0);

    // // Free the sim data because we don't need that any more.
    // wheels_sim_data->free();

    // // Don't forget to add the actor to the scene.
    // {
    //     PxSceneWriteLock scoped_lock(*g_scene_);
    //     g_scene_->addActor(*vehicle_actor);
    // }

    // // Set up the mapping between wheel and actor shape.
    // vehicle->mWheelsSimData.setWheelShapeMapping(0, 0);
    // vehicle->mWheelsSimData.setWheelShapeMapping(1, 1);
    // vehicle->mWheelsSimData.setWheelShapeMapping(2, 2);
    // vehicle->mWheelsSimData.setWheelShapeMapping(3, 3);

    // // Set up the scene query filter data for each suspension line.
    // PxFilterData vehicle_qry_filter_data;
    // vehicle_setup_vehicle_shape_query_filter_data(&vehicle_qry_filter_data);
    // vehicle->mWheelsSimData.setSceneQueryFilterData(0, vehicle_qry_filter_data);
    // vehicle->mWheelsSimData.setSceneQueryFilterData(1, vehicle_qry_filter_data);
    // vehicle->mWheelsSimData.setSceneQueryFilterData(2, vehicle_qry_filter_data);
    // vehicle->mWheelsSimData.setSceneQueryFilterData(3, vehicle_qry_filter_data);

    // // Set the autogear mode of the instantiate vehicle.
    // vehicle->mDriveDynData.setUseAutoGears(use_auto_gear_flag);

    // // Increment the number of vehicles
    // vehicles_[num_vehicles_] = vehicle;
    // vehicle_wheel_query_results_[num_vehicles_].nbWheelQueryResults = 4;
    // vehicle_wheel_query_results_[num_vehicles_].wheelQueryResults = wheel_query_results->add_vehicle(4);
    // num_vehicles_++;

    return new_vehicle;
}