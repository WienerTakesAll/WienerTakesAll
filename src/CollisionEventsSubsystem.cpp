#include <iostream>

#include "CollisionEventsSubsystem.h"

std::vector<std::pair<int, int>> CollisionEventsSubsystem::consume_collisions() {
    auto ret = collisions_;
    collisions_.clear();
    return ret;
}

void CollisionEventsSubsystem::onConstraintBreak(
    physx::PxConstraintInfo* constraints,
    physx::PxU32 count
) {
    // do nothing
}

void CollisionEventsSubsystem::onWake(
    physx::PxActor** actors,
    physx::PxU32 count
) {
    // do nothing
}

void CollisionEventsSubsystem::onSleep(
    physx::PxActor** actors,
    physx::PxU32 count
) {
    // do nothing
}

void CollisionEventsSubsystem::onContact(
    const physx::PxContactPairHeader& pairHeader,
    const physx::PxContactPair* pairs,
    physx::PxU32 nbPairs
) {
    collisions_.emplace_back(0, 0);
}

void CollisionEventsSubsystem::onTrigger(
    physx::PxTriggerPair* pairs,
    physx::PxU32 count
) {
    // do nothing
}

void CollisionEventsSubsystem::onAdvance(
    const physx::PxRigidBody* const* bodyBuffer,
    const physx::PxTransform* poseBuffer,
    const physx::PxU32 count
) {
    // do nothing
}
