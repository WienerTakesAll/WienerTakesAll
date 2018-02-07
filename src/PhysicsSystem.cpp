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
    EventSystem::add_event_handler(EventType::ADD_TERRAIN_EVENT, &PhysicsSystem::handle_add_terrain, this);
   
    PxInitVehicleSDK(*gPhysics_);
    physx::PxVehicleSetBasisVectors(physx::PxVec3(0, 1, 0), physx::PxVec3(0, 0, 1));
    physx::PxVehicleSetUpdateMode(physx::PxVehicleUpdateMode::eVELOCITY_CHANGE);

    physx::PxSceneDesc sceneDesc(gPhysics_->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, 0.05f*-9.81f, 0.0f);

    physx::PxDefaultCpuDispatcher* dispatcher = physx::PxDefaultCpuDispatcherCreate(3);
    sceneDesc.cpuDispatcher = dispatcher;

#if PX_WINDOWS
    // create GPU dispatcher
    physx::PxCudaContextManagerDesc cudaContextManagerDesc;
    auto mCudaContextManager = PxCreateCudaContextManager(*gFoundation_, cudaContextManagerDesc);
    sceneDesc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();
#endif

    sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;

    bool v = sceneDesc.isValid();

    gScene_ = gPhysics_->createScene(sceneDesc);



}

PhysicsSystem::~PhysicsSystem() {
    physx::PxCloseVehicleSDK();

    gCooking_->release();
    gPhysics_->release();
    gFoundation_->release();
}



void PhysicsSystem::update()
{
    if (!gScene_)
        return;

    for (int i = 0; i < 4; i++)
    {
        gScene_->simulate(0.16f/4);
        gScene_->fetchResults(true);
    }

    for (auto& object : dynamic_objects_)
    {
        if (object.is_valid())
        {
            physx::PxTransform transform = object.get_actor()->getGlobalPose();

            EventSystem::queue_event(
                Event(
                    EventType::OBJECT_TRANSFORM_EVENT,
                    "object_id", static_cast<int>(object.get_id()),
                    "pos_x", transform.p.x,
                    "pos_y", transform.p.y,
                    "pos_z", transform.p.z
                )
            );
        }
    }
}



void PhysicsSystem::handle_add_example_ship(const Event& e)
{
    int object_id = e.get_value<int>("object_id", -1);
    assert(object_id != -1);

    physx::PxTransform transform(0.f,0.f,0.f);

    transform.p.x = e.get_value<int>("pos_x", -999);
    assert(transform.p.x != -999);

    transform.p.y = e.get_value<int>("pos_y", -999);
    assert(transform.p.y != -999);

    transform.p.z = e.get_value<int>("pos_z", -999);
    assert(transform.p.z != -999);

    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/Ship.obj");

    dynamic_objects_.emplace_back(object_id);
    dynamic_objects_.back().set_mesh(gPhysics_, gCooking_, mesh);
    dynamic_objects_.back().set_transform(transform);

    gScene_->addActor(*dynamic_objects_.back().get_actor());
}

void PhysicsSystem::handle_add_terrain(const Event& e)
{
    int object_id = e.get_value<int>("object_id", -1);
    assert(object_id != -1);

    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/Terrain.obj");

    static_objects_.emplace_back(object_id);
    static_objects_.back().set_mesh(gPhysics_, gCooking_, mesh);

    gScene_->addActor(*static_objects_.back().get_actor());
}