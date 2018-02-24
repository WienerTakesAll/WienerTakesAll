// #pragma once

// #include "PxPhysicsAPI.h"

// #include "PhysicsComponent.h"
// #include "PhysicsSettings.h"
// #include "WheelMeshGenerator.h"

// // A subsystem of the physics system that manages
// // creating and updating all vehicles
// // Should NOT be tied to the event system
// // All actions come from the PhysicsSystem

// class VehicleSubsystem {
// public:
//     VehicleSubsystem(
//         physx::PxPhysics& physics,
//         physx::PxCooking& cooking
//     );
//     ~VehicleSubsystem();

//     void update(int sim_steps);

//     PhysicsComponent<false> create_4w_vehicle(
//         int id,
//         physx::PxTransform transform,
//         MeshAsset* mesh
//     );

// private:
//     PxPhysics& physics_;
//     PxCooking& cooking_;

//     // Data structures to keep track of vehicles
//     physx::PxVehicleWheels* vehicles_[MAX_NUM_4W_VEHICLES];
//     physx::PxVehicleWheelQueryResult vehicle_wheel_query_results_[MAX_NUM_4W_VEHICLES];
//     physx::PxU32 num_vehicles_;
//     std::vector<VehicleControls> vehicle_controls_;

//     VehicleWheelQueryResults* wheel_query_results_;
//     VehicleSceneQueryData* sq_data_;
//     physx::PxBatchQuery* sq_wheel_raycast_batch_query_;

//     std::vector<PxConvexMesh*> wheel_meshes_template;

//     WheelMeshGenerator wheel_mesh_generator_;
// };
