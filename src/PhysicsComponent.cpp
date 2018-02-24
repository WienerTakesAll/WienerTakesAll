#include "PhysicsComponent.h"

template <>
void PhysicsComponent<true>::create_actor(
    physx::PxPhysics* physics,
    physx::PxTransform& transform,
    physx::PxShape* shape, physx::PxReal density
) {
    g_actor_ = physx::PxCreateStatic(*physics, transform, *shape);
}

template <>
void PhysicsComponent<false>::create_actor(
    physx::PxPhysics* physics,
    physx::PxTransform& transform,
    physx::PxShape* shape,
    physx::PxReal density
) {
    g_actor_ = physics->createRigidDynamic(transform);
}
