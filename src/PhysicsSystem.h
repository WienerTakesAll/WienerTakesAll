#pragma once

#include "PxPhysicsAPI.h"

class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

private:
    physx::PxDefaultAllocator gAllocator_;
    physx::PxDefaultErrorCallback gErrorCallback_;
    physx::PxFoundation* gFoundation_;
};
