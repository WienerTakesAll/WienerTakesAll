#include "PxPhysicsAPI.h"

#include "WheelMeshGenerator.h"

physx::PxConvexMesh* WheelMeshGenerator::create_wheel_convex_mesh(
    const physx::PxVec3* verts,
    const physx::PxU32 num_verts,
    physx::PxPhysics& physics,
    physx::PxCooking& cooking
) {

    //Extract the wheel radius and width from the aabb of the wheel convex mesh.
    physx::PxVec3 wheel_min(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
    physx::PxVec3 wheel_max(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);

    for (physx::PxU32 i = 0; i < num_verts; i++) {
        wheel_min.x = physx::PxMin(wheel_min.x, verts[i].x);
        wheel_min.y = physx::PxMin(wheel_min.y, verts[i].y);
        wheel_min.z = physx::PxMin(wheel_min.z, verts[i].z);
        wheel_max.x = physx::PxMax(wheel_max.x, verts[i].x);
        wheel_max.y = physx::PxMax(wheel_max.y, verts[i].y);
        wheel_max.z = physx::PxMax(wheel_max.z, verts[i].z);
    }

    const physx::PxF32 wheel_width = wheel_max.x - wheel_min.x;
    const physx::PxF32 wheel_radius = physx::PxMax(wheel_max.y, wheel_max.z);

    return create_cylinder_convex_mesh(wheel_width, wheel_radius, 8, physics, cooking);
}


physx::PxConvexMesh* WheelMeshGenerator::create_convex_mesh(
    const physx::PxVec3* verts,
    const physx::PxU32 num_verts,
    physx::PxPhysics& physics,
    physx::PxCooking& cooking
) {

    // Create descriptor for convex mesh
    physx::PxConvexMeshDesc convex_desc;
    convex_desc.points.count = num_verts;
    convex_desc.points.stride = sizeof(physx::PxVec3);
    convex_desc.points.data = verts;
    convex_desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

    physx::PxConvexMesh* convex_mesh = NULL;
    physx::PxDefaultMemoryOutputStream buf;

    if (cooking.cookConvexMesh(convex_desc, buf)) {
        physx::PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
        convex_mesh = physics.createConvexMesh(id);
    }

    return convex_mesh;
}

physx::PxConvexMesh* WheelMeshGenerator::create_cylinder_convex_mesh(
    const physx::PxF32 width,
    const physx::PxF32 radius,
    const physx::PxU32 num_circle_points,
    physx::PxPhysics& physics,
    physx::PxCooking& cooking
) {
#define  MAX_NUM_VERTS_IN_CIRCLE 16
    PX_ASSERT(num_circle_points < MAX_NUM_VERTS_IN_CIRCLE);
    physx::PxVec3 verts[2 * MAX_NUM_VERTS_IN_CIRCLE];
    physx::PxU32 numVerts = 2 * num_circle_points;
    const physx::PxF32 dtheta = 2 * physx::PxPi / (1.0f * num_circle_points);

    for (physx::PxU32 i = 0; i < MAX_NUM_VERTS_IN_CIRCLE; i++) {
        const physx::PxF32 theta = dtheta * i;
        const physx::PxF32 cosTheta = radius * physx::PxCos(theta);
        const physx::PxF32 sinTheta = radius * physx::PxSin(theta);
        verts[2 * i + 0] = physx::PxVec3(-0.5f * width, cosTheta, sinTheta);
        verts[2 * i + 1] = physx::PxVec3(+0.5f * width, cosTheta, sinTheta);
    }

    return create_convex_mesh(verts, numVerts, physics, cooking);

#undef MAX_NUM_VERTS_IN_CIRCLE

}