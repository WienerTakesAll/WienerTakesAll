#pragma once

#include "PxPhysicsAPI.h"

#include "PhysicsComponent.h"
#include "PhysicsSettings.h"
#include "WheelMeshGenerator.h"

// A subsystem of the physics system that manages
// creating and updating all vehicles
// Should NOT be tied to the event system
// All actions come from the PhysicsSystem

class VehiclePhysicsSystem {
public:
    VehiclePhysicsSystem(
        physx::PxPhysics& physics,
        physx::PxCooking& cooking
    );
    ~VehiclePhysicsSystem();

    void update();

    PhysicsComponent<false> create_4w_vehicle(
        int id,
        physx::PxTransform transform,
        MeshAsset* mesh
    );

private:
    PxPhysics& physics_;
    PxCooking& cooking_;



    std::vector<PxConvexMesh*> wheel_meshes_template;

    WheelMeshGenerator wheel_mesh_generator_;
};
