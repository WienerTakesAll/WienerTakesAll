#pragma once

#include "PxPhysicsAPI.h"

#include "EventSystem.h"

class AssetManager;

class PhysicsSystem : public EventSystem<PhysicsSystem> {
public:
    PhysicsSystem(AssetManager&);
    ~PhysicsSystem();

    void update();

private:
    void handle_add_example_ship(const Event& e);

    physx::PxDefaultAllocator gAllocator_;
    physx::PxDefaultErrorCallback gErrorCallback_;
    physx::PxTolerancesScale gScale_;
    physx::PxFoundation* gFoundation_;
    physx::PxPhysics* gPhysics_;
    physx::PxCooking* gCooking_;
    physx::PxScene* gScene_;

    physx::PxRigidActor* gActor_;
    physx::PxMaterial* gMaterial_;
    physx::PxTriangleMesh* gTestMesh_;
    physx::PxShape* gMeshShape_;
    physx::PxRigidDynamic* gTestObject_;

    AssetManager& asset_manager_;
};
