#include <iostream>

#include "CollisionEventsSubsystem.h"

CollisionEventsSubsystem::CollisionEventsSubsystem() {
    std::cout << "CollisionEventsSubsystem" << std::endl;
}

void CollisionEventsSubsystem::onConstraintBreak(
    physx::PxConstraintInfo* constraints,
    physx::PxU32 count
) {
    std::cout << "onConstraintBreak" << std::endl;
}
void CollisionEventsSubsystem::onWake(
    physx::PxActor** actors,
    physx::PxU32 count
) {
    std::cout << "onWake" << std::endl;
}
void CollisionEventsSubsystem::onSleep(
    physx::PxActor** actors,
    physx::PxU32 count
) {
    std::cout << "onSleep" << std::endl;
}
void CollisionEventsSubsystem::onContact(
    const physx::PxContactPairHeader& pairHeader,
    const physx::PxContactPair* pairs,
    physx::PxU32 nbPairs
) {
    std::cout << "onContact" << std::endl;
}
void CollisionEventsSubsystem::onTrigger(
    physx::PxTriggerPair* pairs,
    physx::PxU32 count
) {
    std::cout << "onTrigger" << std::endl;
}
void CollisionEventsSubsystem::onAdvance(
    const physx::PxRigidBody* const* bodyBuffer,
    const physx::PxTransform* poseBuffer,
    const physx::PxU32 count
) {
    std::cout << "onAdvance" << std::endl;
}