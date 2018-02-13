#pragma once

#include <type_traits>

#include "PxPhysicsAPI.h"
#include "MeshAsset.h"


enum {
    COLLISION_FLAG_GROUND = 1 << 0,
    COLLISION_FLAG_WHEEL = 1 << 1,
    COLLISION_FLAG_CHASSIS = 1 << 2,
    COLLISION_FLAG_OBSTACLE = 1 << 3,
    COLLISION_FLAG_DRIVABLE_OBSTACLE = 1 << 4,

    COLLISION_FLAG_GROUND_AGAINST = COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
    COLLISION_FLAG_WHEEL_AGAINST = COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
    COLLISION_FLAG_CHASSIS_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
    COLLISION_FLAG_OBSTACLE_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
    COLLISION_FLAG_DRIVABLE_OBSTACLE_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
};

//Id of drivable surface (used by suspension raycast filtering).
enum {
    DRIVABLE_SURFACE_ID = 0xffffffff
};

//Drivable surface types.
enum {
    SURFACE_TYPE_MUD = 0,
    SURFACE_TYPE_TARMAC,
    SURFACE_TYPE_SNOW,
    SURFACE_TYPE_GRASS,
    MAX_NUM_SURFACE_TYPES
};

//Tire types.
enum {
    TIRE_TYPE_WETS = 0,
    TIRE_TYPE_SLICKS,
    TIRE_TYPE_ICE,
    TIRE_TYPE_MUD,
    MAX_NUM_TIRE_TYPES
};

enum {
    SAMPLEVEHICLE_DRIVABLE_SURFACE = 0xffff0000,
    SAMPLEVEHICLE_UNDRIVABLE_SURFACE = 0x0000ffff
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