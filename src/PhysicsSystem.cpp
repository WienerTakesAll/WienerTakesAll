#include "PhysicsSystem.h"

#include "AssetManager.h"
#include "SDL.h"

#include <assert.h>
#include <glm/gtx/quaternion.hpp>

const physx::PxVec3 GRAVITY(0.0f, 0.05f*-9.81f, 0.0f);



void setupDrivableSurface(physx::PxFilterData& filterData)
{
    filterData.word3 = static_cast<physx::PxU32>(CollisionFlags::DRIVABLE_SURFACE);
}

void setupNonDrivableSurface(physx::PxFilterData& filterData)
{
    filterData.word3 = static_cast<physx::PxU32>(CollisionFlags::UNDRIVABLE_SURFACE);
}

physx::PxQueryHitType::Enum WheelRaycastPreFilter
(physx::PxFilterData filterData0, physx::PxFilterData filterData1,
    const void* constantBlock, physx::PxU32 constantBlockSize,
    physx::PxHitFlags& queryFlags)
{
    //filterData0 is the vehicle suspension raycast.
    //filterData1 is the shape potentially hit by the raycast.
    PX_UNUSED(constantBlockSize);
    PX_UNUSED(constantBlock);
    PX_UNUSED(filterData0);
    PX_UNUSED(queryFlags);
    return ((0 == (filterData1.word3 & static_cast<physx::PxU32>(CollisionFlags::DRIVABLE_SURFACE))) ?
        physx::PxQueryHitType::eNONE : physx::PxQueryHitType::eBLOCK);
}

physx::PxVehicleKeySmoothingData gKeySmoothingData =
{
    {
        3.0f,    //rise rate eANALOG_INPUT_ACCEL
        3.0f,    //rise rate eANALOG_INPUT_BRAKE
        10.0f,    //rise rate eANALOG_INPUT_HANDBRAKE
        2.5f,    //rise rate eANALOG_INPUT_STEER_LEFT
        2.5f,    //rise rate eANALOG_INPUT_STEER_RIGHT
    },
    {
        5.0f,    //fall rate eANALOG_INPUT__ACCEL
        5.0f,    //fall rate eANALOG_INPUT__BRAKE
        10.0f,    //fall rate eANALOG_INPUT__HANDBRAKE
        5.0f,    //fall rate eANALOG_INPUT_STEER_LEFT
        5.0f    //fall rate eANALOG_INPUT_STEER_RIGHT
    }
};

physx::PxVehiclePadSmoothingData gPadSmoothingData =
{
    {
        6.0f,    //rise rate eANALOG_INPUT_ACCEL
        6.0f,    //rise rate eANALOG_INPUT_BRAKE
        12.0f,    //rise rate eANALOG_INPUT_HANDBRAKE
        2.5f,    //rise rate eANALOG_INPUT_STEER_LEFT
        2.5f,    //rise rate eANALOG_INPUT_STEER_RIGHT
    },
    {
        10.0f,    //fall rate eANALOG_INPUT_ACCEL
        10.0f,    //fall rate eANALOG_INPUT_BRAKE
        12.0f,    //fall rate eANALOG_INPUT_HANDBRAKE
        5.0f,    //fall rate eANALOG_INPUT_STEER_LEFT
        5.0f    //fall rate eANALOG_INPUT_STEER_RIGHT
    }
};

physx::PxF32 gSteerVsForwardSpeedData[2 * 8] =
{
    0.0f,        0.75f,
    5.0f,        0.75f,
    30.0f,        0.125f,
    120.0f,        0.1f,
    PX_MAX_F32, PX_MAX_F32,
    PX_MAX_F32, PX_MAX_F32,
    PX_MAX_F32, PX_MAX_F32,
    PX_MAX_F32, PX_MAX_F32
};
physx::PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData, 4);



PhysicsSystem::PhysicsSystem(AssetManager& asset_manager)
    : gFoundation_(PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator_, gErrorCallback_))
    , gScale_()
    , gPhysics_(PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation_, gScale_))
    , gCooking_(PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation_, gScale_))
    , gScene_(nullptr)
    , asset_manager_(asset_manager) 
    , forwardDrive(0.0f), horizontalDrive(0.0f) {
    EventSystem::add_event_handler(EventType::ADD_EXAMPLE_SHIP_EVENT, &PhysicsSystem::handle_add_example_ship, this);
    EventSystem::add_event_handler(EventType::ADD_TERRAIN_EVENT, &PhysicsSystem::handle_add_terrain, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &PhysicsSystem::handle_key_press, this);


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
    
    using namespace physx;

    std::vector<physx::PxVehicleWheels*> wheels;

    for (auto& object : dynamic_objects_) {
        auto pose = object.get_actor()->getGlobalPose();
        auto rotate = glm::toMat4(glm::quat(pose.q.w,pose.q.x,pose.q.y,pose.q.z));

        object.get_actor()->addForce(physx::PxVec3(rotate[2][0]*forwardDrive, 0.f, rotate[2][2]*forwardDrive));
        object.get_actor()->addTorque(physx::PxVec3(0.f, horizontalDrive*0.1f, 0.f));

        if (object.get_wheels()) {
            wheels.push_back(object.get_wheels());
            break;
        }
    }


    for (int i = 0; i < 1; i++)
    {
        if (wheels.size()) {
            frictionPairs = createFrictionPairs(dynamic_objects_[0].get_material());

            physx::PxVehicleDrive4WRawInputData gVehicleInputData;
            gVehicleInputData.setAnalogAccel(1.0f);
            gVehicleInputData.setAnalogBrake(0.0f);
            gVehicleInputData.setAnalogHandbrake(0.0f);
            gVehicleInputData.setAnalogSteer(1.0f);
            gVehicleInputData.setGearUp(1.0f);
            gVehicleInputData.setGearDown(0.0f);

            auto actor = dynamic_objects_[0].get_wheels();
            physx::PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData,
                gSteerVsForwardSpeedTable, gVehicleInputData, 0.16f, false, *actor);

            physx::PxRaycastQueryResult sqResults[4];
            physx::PxRaycastHit sqHitBuffer[4];
            physx::PxBatchQueryDesc sqDesc(4, 0, 0);
            sqDesc.queryMemory.userRaycastResultBuffer = sqResults;
            sqDesc.queryMemory.userRaycastTouchBuffer = sqHitBuffer;
            sqDesc.queryMemory.raycastTouchBufferSize = 4;
            sqDesc.preFilterShader = &WheelRaycastPreFilter;
            physx::PxBatchQuery* batchQuery = gScene_->createBatchQuery(sqDesc);

            PxVehicleSuspensionRaycasts(batchQuery, 1, &wheels[0], 4, sqResults);
            physx::PxVehicleUpdates(0.16f, GRAVITY, *frictionPairs, 1, &wheels[0], NULL);
        }
        gScene_->simulate(0.16f);
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
                    "pos_z", transform.p.z,
                    "qua_w", transform.q.w,
                    "qua_x", transform.q.x,
                    "qua_y", transform.q.y,
                    "qua_z", transform.q.z
                )
            );
        }
    }
}

void PhysicsSystem::handle_key_press(const Event& e) {
    int player_id = e.get_value<int>("player_id", -1);
    int key = e.get_value<int>("key", -1);
    int value = e.get_value<int>("value", 0);

    switch (key) {

    case SDL_CONTROLLER_AXIS_LEFTY:
        forwardDrive = (float)value / -32768;
        break; 

    case SDL_CONTROLLER_AXIS_LEFTX:
        horizontalDrive = (float)value / -32768;
        break; 

    default:
        break;
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

    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/carBoxModel.obj");

    dynamic_objects_.emplace_back(object_id);
    dynamic_objects_.back().create_vehicle(gPhysics_, gCooking_, mesh);
    dynamic_objects_.back().set_transform(transform);

    auto actor = dynamic_objects_.back().get_actor();
    gScene_->addActor(*actor);
    
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

    PxU32 SURFACE_TYPE_TARMAC = static_cast<PxU32>(CollisionFlags::DRIVABLE_SURFACE);
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