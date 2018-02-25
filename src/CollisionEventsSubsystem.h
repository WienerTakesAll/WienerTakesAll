#pragma once

#include "PxPhysicsAPI.h"

class CollisionEventsSubsystem : public physx::PxSimulationEventCallback {
public:
    CollisionEventsSubsystem();

    void onConstraintBreak(
        physx::PxConstraintInfo* constraints,
        physx::PxU32 count
    );
    void onWake(
        physx::PxActor** actors,
        physx::PxU32 count
    );
    void onSleep(
        physx::PxActor** actors,
        physx::PxU32 count
    );
    void onContact(
        const physx::PxContactPairHeader& pairHeader,
        const physx::PxContactPair* pairs,
        physx::PxU32 nbPairs
    );
    void onTrigger(
        physx::PxTriggerPair* pairs,
        physx::PxU32 count
    );
    void onAdvance(
        const physx::PxRigidBody* const* bodyBuffer,
        const physx::PxTransform* poseBuffer,
        const physx::PxU32 count
    );
};