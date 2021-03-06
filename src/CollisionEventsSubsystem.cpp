#include <iostream>

#include "CollisionEventsSubsystem.h"

CollisionEventsSubsystem::CollisionEventsSubsystem()
    : delay (0) {
}

std::vector<std::pair<int, int>> CollisionEventsSubsystem::consume_collisions() {
    if (--delay < 0) {
        delay = 0;
    }

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
    if (delay > 0) {
        return;
    }

    delay = 10;

    int a_id = *(int*)pairHeader.actors[0]->userData;
    int b_id = *(int*)pairHeader.actors[1]->userData;

    collisions_.emplace_back(a_id, b_id);
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
