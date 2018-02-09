#pragma once

#include "PxPhysicsAPI.h"
#include "EventSystem.h"
#include "PhysicsComponent.h"

class AssetManager;

class PhysicsSystem : public EventSystem<PhysicsSystem> {
public:
    PhysicsSystem(AssetManager&);
    ~PhysicsSystem();

    void update();

private:
    void handle_add_example_ship(const Event& e);
    void handle_add_terrain(const Event& e);
    void handle_key_press(const Event& e);

    physx::PxDefaultAllocator gAllocator_;
    physx::PxDefaultErrorCallback gErrorCallback_;
    physx::PxTolerancesScale gScale_;
    physx::PxFoundation* gFoundation_;
    physx::PxPhysics* gPhysics_;
    physx::PxCooking* gCooking_;
    physx::PxScene* gScene_;

    AssetManager& asset_manager_;

    std::vector<PhysicsComponent<false>> dynamic_objects_;
    std::vector<PhysicsComponent<true>> static_objects_;

    physx::PxVehicleDrivableSurfaceToTireFrictionPairs* frictionPairs;
    physx::PxVehicleDrivableSurfaceToTireFrictionPairs* createFrictionPairs(const physx::PxMaterial* defaultMaterial);
};