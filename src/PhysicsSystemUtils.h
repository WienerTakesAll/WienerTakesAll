#pragma once

#include "PxPhysicsAPI.h"

using namespace physx;

void setup_actor (
    PxRigidDynamic* veh_actor,
    const PxFilterData& veh_qry_filter_data,
    const PxGeometry** wheel_geometries,
    const PxTransform* wheel_local_poses,
    const PxU32 num_wheel_geometries,
    const PxMaterial* wheel_material,
    const PxFilterData& wheel_coll_filterData,
    const PxGeometry** chassis_geometries,
    const PxTransform* chassis_local_poses,
    const PxU32 num_chassis_geometries,
    const PxMaterial* chassis_material,
    const PxFilterData& chassis_coll_filter_data,
    const PxVehicleChassisData& chassis_data,
    PxPhysics* physics
);

PxRigidDynamic* create_4w_vehicle_actor(
    const PxVehicleChassisData& chassis_data,
    PxConvexMesh** wheel_convex_meshes,
    PxConvexMesh* chassis_convex_mesh,
    PxScene& scene,
    PxPhysics& physics,
    const PxMaterial& material
);

void compute_wheel_widths_and_radii(
    PxConvexMesh** wheel_convex_meshes,
    PxF32* wheel_widths,
    PxF32* wheel_radii
);

void vehicle_setup_vehicle_shape_query_filter_data(PxFilterData* qry_filter_data);

PxVehicleChassisData create_chassis_data(
    const PxF32 chassis_mass,
    PxConvexMesh* chassis_convex_mesh
);

PxVehicleDriveSimData4W create_drive_sim_data(
    const PxVec3 wheel_centre_offsets[4]
);

PxVehicleWheelsSimData* create_wheels_sim_data(
    const PxVehicleChassisData& chassis_data,
    const PxF32 wheel_mass,
    PxConvexMesh** wheel_convex_meshes,
    const PxVec3* wheel_centre_offsets
);

PxFilterFlags wienerTakesAllFilterShader(
    PxFilterObjectAttributes attributes0,
    PxFilterData filterData0,
    PxFilterObjectAttributes attributes1,
    PxFilterData filterData1,
    PxPairFlags& pairFlags,
    const void* constantBlock,
    PxU32 constantBlockSize
);