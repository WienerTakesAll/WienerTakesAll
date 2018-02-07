#include "PhysicsComponent.h"

template <>
void PhysicsComponent<true>::createActor(physx::PxPhysics* physics, physx::PxTransform& transform, physx::PxShape* shape, physx::PxReal density) {
    gActor_ = physx::PxCreateStatic(*physics, transform, *gMeshShape_);
}

template <>
void PhysicsComponent<false>::createActor(physx::PxPhysics* physics, physx::PxTransform& transform, physx::PxShape* shape, physx::PxReal density) {
    gActor_ = physx::PxCreateDynamic(*physics, transform, *gMeshShape_, density);
}