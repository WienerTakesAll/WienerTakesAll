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

    void set_mesh(physx::PxPhysics* physics, physx::PxCooking* cooking, MeshAsset* mesh);
    void set_transform(physx::PxTransform& transform);
private:
    void createActor(physx::PxPhysics* physics, physx::PxTransform& transform, physx::PxShape* shape, physx::PxReal density);
   
    bool valid_;
    unsigned int id_;

    physx::PxMaterial* gMaterial_;
    physx::PxConvexMesh* gMesh_;
    physx::PxShape* gMeshShape_;
    PxActorType* gActor_;
};


#include "PhysicsComponent_impl.h"