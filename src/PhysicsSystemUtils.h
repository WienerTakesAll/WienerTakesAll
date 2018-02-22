#pragma once

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

PxVec3 compute_chassis_aabb_dimensions(const PxConvexMesh* chassis_convex_mesh);

void create_4w_vehicle_simulation_data(
    const PxF32 chassis_mass,
    PxConvexMesh* chassis_convex_mesh,
    const PxF32 wheel_mass,
    PxConvexMesh** wheel_convex_meshes,
    const PxVec3* wheel_centre_offsets,
    PxVehicleWheelsSimData& wheels_data,
    PxVehicleDriveSimData4W& drive_data,
    PxVehicleChassisData& chassis_data
);

PxConvexMesh* create_convex_mesh(
    const PxVec3* verts,
    const PxU32 numVerts,
    PxPhysics& physics,
    PxCooking& cooking
);

PxConvexMesh* create_cylinder_convex_mesh(
    const PxF32 width,
    const PxF32 radius,
    const PxU32 num_circle_points,
    PxPhysics& physics,
    PxCooking& cooking
);

PxConvexMesh* create_wheel_convex_mesh(
    const PxVec3* verts,
    const PxU32 numVerts,
    PxPhysics& physics,
    PxCooking& cooking
);

void vehicle_setup_vehicle_shape_query_filter_data(PxFilterData* qry_filter_data);
