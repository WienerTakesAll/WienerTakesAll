#include "PhysicsSystem.h"

#include "AssetManager.h"

#include <assert.h>

PhysicsSystem::PhysicsSystem(AssetManager& asset_manager)
    : gFoundation_(PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator_, gErrorCallback_))
    , gScale_()
    , gPhysics_(PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation_, gScale_))
    , gCooking_(PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation_, gScale_))
    , gScene_(nullptr)
    , asset_manager_(asset_manager) {
    EventSystem::add_event_handler(EventType::ADD_EXAMPLE_SHIP_EVENT, &PhysicsSystem::handle_add_example_ship, this);
   
    auto tol = gPhysics_->getTolerancesScale();

    physx::PxSceneDesc sceneDesc(tol);
    sceneDesc.setToDefault(gScale_);

    bool v = gPhysics_->getTolerancesScale().isValid();

    gScene_ = gPhysics_->createScene(sceneDesc);
    assert(gScene_ != nullptr);

}

PhysicsSystem::~PhysicsSystem() {
    gCooking_->release();
    gPhysics_->release();
    gFoundation_->release();
}



void PhysicsSystem::update()
{
    if (gScene_ == nullptr)
        return;

    gScene_->simulate(0.16f);
    gScene_->fetchResults(true);

    physx::PxVec3 vel = gTestObject_->getLinearVelocity();
    std::cout << vel.x << " " << vel.y << " " << vel.z << std::endl;
}



void PhysicsSystem::handle_add_example_ship(const Event& e)
{

    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/Ship.obj");

    physx::PxTriangleMeshDesc meshDesc;
    physx::PxStridedData data;

    std::vector<physx::PxVec3> physVerts;
    std::vector<physx::PxU32> physIndices;
    
    for (auto& vert : mesh->vertices_) {
        physx::PxVec3 point;

        point.x = vert.position_[0];
        point.y = vert.position_[1];
        point.z = vert.position_[2];

        physVerts.push_back(point);
    }

    for (auto& ind : mesh->indices_) {
        physx::PxU32 index;

        index = ind;

        physIndices.push_back(index);
    }

    meshDesc.points.count = physVerts.size();
    meshDesc.points.data = &physVerts.front();
    meshDesc.points.stride = sizeof(physx::PxVec3);
    
    meshDesc.triangles.count = physIndices.size();
    meshDesc.triangles.data = &physIndices.front();
    meshDesc.triangles.stride = sizeof(physx::PxU32);

    physx::PxDefaultMemoryOutputStream writeBuffer;
    physx::PxTriangleMeshCookingResult::Enum result;
    bool status = gCooking_->cookTriangleMesh(meshDesc, writeBuffer, &result);
    if (!status)
    {
        std::cerr << "Could not create Physx mesh";
        return;
    }

    physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    gTestMesh_ = gPhysics_->createTriangleMesh(readBuffer);

    physx::PxTransform physTransform(0, 0, 0);
    gMaterial_ = gPhysics_->createMaterial(0.1f, 0.1f, 0.1f);

    physx::PxTriangleMeshGeometry meshGeometry(gTestMesh_);

    gMeshShape_ = physx::PxRigidActorExt::createExclusiveShape(*gActor_, meshGeometry, *gMaterial_);

    gTestObject_ = physx::PxCreateDynamic(*gPhysics_, physTransform, *gMeshShape_, 1.0f);

    gScene_->addActor(*gTestObject_);

    return;
}