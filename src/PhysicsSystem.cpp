#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
    : gFoundation_(PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator_, gErrorCallback_)) {
}

PhysicsSystem::~PhysicsSystem() {

}
