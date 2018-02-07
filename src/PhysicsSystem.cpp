#include "PhysicsSystem.h"

#include "AssetManager.h"

#include <assert.h>

const physx::PxVec3 GRAVITY(0.0f, 0.05f*-9.81f, 0.0f);

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
    sceneDesc.gravity = GRAVITY;

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

    std::vector<physx::PxVehicleWheels*> wheels;

    for (auto& object : dynamic_objects_) {
        if (object.get_wheels()) {
            wheels.push_back(object.get_wheels());
        }
    }

    for (int i = 0; i < 4; i++)
    {
        gScene_->simulate(0.16f/4);
        physx::PxVehicleUpdates(0.16f/4, GRAVITY, *frictionPairs, 2, &wheels.front(), NULL);
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
    dynamic_objects_.back().create_vehicle(gPhysics_, gCooking_, mesh);
    dynamic_objects_.back().set_transform(transform);

    auto actor = dynamic_objects_.back().get_actor();
    gScene_->addActor(*actor);
    frictionPairs = createFrictionPairs(dynamic_objects_.back().get_material());

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




physx::PxVehicleDrivableSurfaceToTireFrictionPairs* PhysicsSystem::createFrictionPairs (const physx::PxMaterial* defaultMaterial) {
    using namespace physx;

    PxU32 SURFACE_TYPE_TARMAC = 1;
    PxU32 MAX_NUM_TIRE_TYPES = 1;
    PxU32 MAX_NUM_SURFACE_TYPES = 1;
    
    PxReal gTireFrictionMultipliers[1][1] = { {0.5f} };

    PxVehicleDrivableSurfaceType surfaceTypes[1];
    surfaceTypes[0].mType = SURFACE_TYPE_TARMAC;

    const PxMaterial* surfaceMaterials[1];
    surfaceMaterials[0] = defaultMaterial;

    PxVehicleDrivableSurfaceToTireFrictionPairs* surfaceTirePairs =
        PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(MAX_NUM_TIRE_TYPES,
            MAX_NUM_SURFACE_TYPES);

    surfaceTirePairs->setup(MAX_NUM_TIRE_TYPES, MAX_NUM_SURFACE_TYPES, surfaceMaterials, surfaceTypes);

    for (PxU32 i = 0; i < MAX_NUM_SURFACE_TYPES; i++)
    {
        for (PxU32 j = 0; j < MAX_NUM_TIRE_TYPES; j++)
        {
            surfaceTirePairs->setTypePairFriction(i, j, gTireFrictionMultipliers[i][j]);
        }
    }
    return surfaceTirePairs;
}