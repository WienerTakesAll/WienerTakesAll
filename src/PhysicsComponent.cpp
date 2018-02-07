#include "PhysicsComponent.h"

template <>
void PhysicsComponent<true>::createActor(physx::PxPhysics* physics, physx::PxTransform& transform, physx::PxShape* shape, physx::PxReal density) {
    gActor_ = physics->createRigidStatic(transform);
}

template <>
void PhysicsComponent<false>::createActor(physx::PxPhysics* physics, physx::PxTransform& transform, physx::PxShape* shape, physx::PxReal density) {
    gActor_ = physics->createRigidDynamic(transform);
}