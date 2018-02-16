#include "PhysicsComponent.h"

template <>
void PhysicsComponent<true>::createActor(physx::PxPhysics* physics, physx::PxTransform& transform, physx::PxShape* shape, physx::PxReal density) {
    gActor_ = physx::PxCreateStatic(*physics, transform, *shape);
    //gActor_ = physics->createRigidStatic(transform);
}

template <>
void PhysicsComponent<false>::createActor(physx::PxPhysics* physics, physx::PxTransform& transform, physx::PxShape* shape, physx::PxReal density) {
    gActor_ = physics->createRigidDynamic(transform);
}