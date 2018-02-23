#pragma once

#include "PxPhysicsAPI.h"

class WheelMeshGenerator {
public:
    physx::PxConvexMesh* create_wheel_convex_mesh(
        const physx::PxVec3* verts,
        const physx::PxU32 num_verts,
        physx::PxPhysics& physics,
        physx::PxCooking& cooking
    );

private:
    physx::PxConvexMesh* create_convex_mesh(
        const physx::PxVec3* verts,
        const physx::PxU32 num_verts,
        physx::PxPhysics& physics,
        physx::PxCooking& cooking
    );

    physx::PxConvexMesh* create_cylinder_convex_mesh(
        const physx::PxF32 width,
        const physx::PxF32 radius,
        const physx::PxU32 num_circle_points,
        physx::PxPhysics& physics,
        physx::PxCooking& cooking
    );
};
