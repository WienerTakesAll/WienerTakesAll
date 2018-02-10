#pragma once

#include <type_traits>

#include "PxPhysicsAPI.h"
#include "MeshAsset.h"

enum class CollisionFlags
{
    WHEELS = (1 << 0),
    TERRAIN = (1 << 1),


    COLLISION_WHEEL_AGAINST = TERRAIN,
    UNDRIVABLE_SURFACE = 0,
    DRIVABLE_SURFACE = (1 << 8)
};

template <bool static_actor>
class PhysicsComponent {
public:
    using PxActorType = typename std::conditional<static_actor, physx::PxRigidStatic, physx::PxRigidDynamic>::type;
    
    PhysicsComponent(unsigned int id);
    ~PhysicsComponent();

    bool is_valid();
    unsigned int get_id();
    auto get_actor();
    void set_actor(physx::PxRigidDynamic* actor);

    physx::PxMaterial* get_material();
    physx::PxConvexMesh* get_mesh();

    physx::PxVehicleDrive4W* get_wheels();

    void set_mesh(physx::PxPhysics* physics, physx::PxCooking* cooking, MeshAsset* mesh);
    void create_vehicle(physx::PxPhysics* physics, physx::PxCooking* cooking, MeshAsset* mesh);
    void set_transform(physx::PxTransform& transform);
private:
    void createActor(physx::PxPhysics* physics, physx::PxTransform& transform, physx::PxShape* shape, physx::PxReal density);
   
    void setup_wheels(physx::PxVehicleWheelsSimData* wheelsSimData);
    void setup_drive_sim(physx::PxVehicleDriveSimData4W& driveSimData, physx::PxVehicleWheelsSimData* wheelsSimData);

    bool valid_;
    unsigned int id_;

    physx::PxMaterial* gMaterial_;
    physx::PxConvexMesh* gMesh_;
    physx::PxConvexMeshGeometry* gMeshGeometry_;
    physx::PxShape* gMeshShape_;
    PxActorType* gActor_;

    bool is_vehicle_;
    physx::PxVehicleDrive4W* gDrive4W_;
};


#include "PhysicsComponent_impl.h"