#pragma once

#include <type_traits>

#include "PxPhysicsAPI.h"
#include "MeshAsset.h"

template <bool static_actor>
class PhysicsComponent {
public:
    using PxActorType = typename std::conditional<static_actor, physx::PxRigidStatic, physx::PxRigidDynamic>::type;
    using PxMeshType = typename std::conditional<static_actor, physx::PxTriangleMesh, physx::PxConvexMesh>::type;
    using PxMeshGeometryType = typename std::conditional<static_actor, physx::PxTriangleMeshGeometry, physx::PxConvexMeshGeometry>::type;

    PhysicsComponent(unsigned int id);
    ~PhysicsComponent();

    bool is_valid() const;
    unsigned int get_id() const;
    physx::PxMaterial* get_material() const;
    PxMeshType* get_mesh() const;
    PxMeshGeometryType* get_mesh_geometry() const;
    physx::PxShape* get_mesh_shape() const;
    auto get_actor() const;
    bool is_vehicle() const;
    physx::PxVehicleDrive4W* get_wheels() const;

    void set_is_vehicle(bool is_vehicle);
    void set_actor(physx::PxRigidDynamic* actor);
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
        physx::PxReal density
    );

    void create_geometry(
        physx::PxPhysics* physics,
        physx::PxCooking* cooking,
        MeshAsset* mesh
    );

    void setup_wheels(physx::PxVehicleWheelsSimData* wheelsSimData);
    void setup_drive_sim(physx::PxVehicleDriveSimData4W& driveSimData, physx::PxVehicleWheelsSimData* wheelsSimData);

    bool valid_;
    unsigned int id_;
    int* id_ptr_;

    physx::PxMaterial* g_material_;
    PxMeshType* g_mesh_;
    PxMeshGeometryType* g_mesh_geometry_;
    physx::PxShape* g_mesh_shape_;
    PxActorType* g_actor_;

    bool is_vehicle_;
    physx::PxVehicleDrive4W* g_drive_4w_;
};

#include "PhysicsComponent_impl.h"
