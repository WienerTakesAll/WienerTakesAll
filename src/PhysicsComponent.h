#pragma once

#include <type_traits>

#include "PxPhysicsAPI.h"
#include "MeshAsset.h"

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

    void set_mesh(
        physx::PxPhysics* physics,
        physx::PxCooking* cooking,
        MeshAsset* mesh);
    void create_vehicle(
        physx::PxPhysics* physics,
        physx::PxCooking* cooking,
        MeshAsset* mesh);
    void set_transform(physx::PxTransform& transform);

    void set_material(physx::PxMaterial* material);
private:
    void create_actor(
        physx::PxPhysics* physics,
        physx::PxTransform& transform,
        physx::PxShape* shape,
        physx::PxReal density);

    void setup_wheels(physx::PxVehicleWheelsSimData* wheelsSimData);
    void setup_drive_sim(physx::PxVehicleDriveSimData4W& driveSimData, physx::PxVehicleWheelsSimData* wheelsSimData);

    bool valid_;
    unsigned int id_;

    physx::PxMaterial* g_material_;
    physx::PxConvexMesh* g_mesh_;
    physx::PxConvexMeshGeometry* g_mesh_geometry_;
    physx::PxShape* g_mesh_shape_;
    PxActorType* g_actor_;

    bool is_vehicle_;
    physx::PxVehicleDrive4W* g_drive_4w_;
};

#include "PhysicsComponent_impl.h"