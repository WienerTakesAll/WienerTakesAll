#include <assert.h>
#include <stdio.h>

#include <SDL.h>
#include <glm/gtx/quaternion.hpp>
#include "vehicle/PxVehicleTireFriction.h"
#include "vehicle/PxVehicleWheels.h"

#include "PhysicsSystem.h"
#include "AssetManager.h"
#include "PhysicsSystem.h"
#include "PhysicsSettings.h"
#include "MeshStuff.h"

using namespace physx;


PhysicsSystem::PhysicsSystem(AssetManager& asset_manager, PhysicsSettings& physics_settings)
    : g_foundation_(PxCreateFoundation(PX_FOUNDATION_VERSION, g_allocator_, g_error_callback_))
    , g_scale_()
    , g_pvd_(PxCreatePvd(*g_foundation_))
    , g_physics_(PxCreatePhysics(PX_PHYSICS_VERSION, *g_foundation_, g_scale_, false, g_pvd_))
    , g_cooking_(PxCreateCooking(PX_PHYSICS_VERSION, *g_foundation_, g_scale_))
    , g_scene_(NULL)
    , sq_wheel_raycast_batch_query_(NULL)
    , asset_manager_(asset_manager)
    , forward_drive_(0.0f)
    , horizontal_drive_(0.0f)
    , backward_drive_(0.0f)
    , hand_break_(false)
    , num_vehicles_(0)
      // Allocate simulation data so we can switch from 3-wheeled to 4-wheeled cars by switching simulation data.
    , wheels_sim_data_4w_ (PxVehicleWheelsSimData::allocate(4))
      // Scene query data for to allow raycasts for all suspensions of all vehicles.
    , sq_data_ (SampleVehicleSceneQueryData::allocate(MAX_NUM_4W_VEHICLES * 4))
      // Data to store reports for each wheel.
    , wheel_query_results (SampleVehicleWheelQueryResults::allocate(MAX_NUM_4W_VEHICLES * 4))
    , settings_(physics_settings) {

    EventSystem::add_event_handler(EventType::ADD_EXAMPLE_SHIP_EVENT, &PhysicsSystem::handle_add_example_ship, this);
    EventSystem::add_event_handler(EventType::ADD_TERRAIN_EVENT, &PhysicsSystem::handle_add_terrain, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &PhysicsSystem::handle_key_press, this);

    // Setup Visual Debugger
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    g_pvd_->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // Vehicle Physics Initialization
    PxInitVehicleSDK(*g_physics_);
    physx::PxVehicleSetBasisVectors(physx::PxVec3(0, 1, 0), physx::PxVec3(0, 0, 1));
    physx::PxVehicleSetUpdateMode(physx::PxVehicleUpdateMode::eVELOCITY_CHANGE);

    // Scene Initialization
    physx::PxSceneDesc sceneDesc(g_physics_->getTolerancesScale());
    sceneDesc.gravity = settings_.gravity;

    physx::PxDefaultCpuDispatcher* dispatcher = physx::PxDefaultCpuDispatcherCreate(3);
    sceneDesc.cpuDispatcher = dispatcher;

#if PX_WINDOWS
    physx::PxCudaContextManagerDesc cudaContextManagerDesc;
    auto mCudaContextManager = PxCreateCudaContextManager(*g_foundation_, cudaContextManagerDesc);
    sceneDesc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();
#endif

    sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
    assert(sceneDesc.isValid());
    g_scene_ = g_physics_->createScene(sceneDesc);

    // Initialise all vehicle ptrs to null.
    for (PxU32 i = 0; i < MAX_NUM_4W_VEHICLES; i++) {
        vehicles_[i] = NULL;
    }
}

PhysicsSystem::~PhysicsSystem() {
    physx::PxCloseVehicleSDK();

    g_cooking_->release();
    g_physics_->release();
    g_pvd_->release();
    g_foundation_->release();
}

void PhysicsSystem::update() {
    assert(g_scene_);

    const int SIM_STEPS = 4;

    for (int i = 0; i < SIM_STEPS; i++) {
        if (!num_vehicles_) {
            g_scene_->simulate(0.16f / SIM_STEPS);
            g_scene_->fetchResults(true);
            continue;
        }

        // Build vehicle input data
        physx::PxVehicleDrive4WRawInputData gVehicleInputData;
        gVehicleInputData.setDigitalAccel(true);
        gVehicleInputData.setAnalogAccel(std::max(forward_drive_, 0.f));
        gVehicleInputData.setAnalogBrake(backward_drive_);
        gVehicleInputData.setAnalogHandbrake(hand_break_ * 1.0f);
        hand_break_ = false;
        gVehicleInputData.setAnalogSteer(horizontal_drive_);

        PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(
            settings_.g_pad_smoothing_data,
            settings_.g_steer_vs_forward_speed_table,
            gVehicleInputData,
            0.16f / SIM_STEPS,
            false,
            (PxVehicleDrive4W&)*vehicles_[0]
        );

        if (NULL == sq_wheel_raycast_batch_query_) {
            sq_wheel_raycast_batch_query_ = sq_data_->setup_batched_scene_query(g_scene_);
        }

        PxVehicleSuspensionRaycasts(
            sq_wheel_raycast_batch_query_,
            num_vehicles_,
            vehicles_,
            sq_data_->get_raycast_query_result_buffer_size(),
            sq_data_->get_raycast_query_result_buffer()
        );

        PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];
        PxVehicleWheelQueryResult vehicleQueryResults[1] = {
            {
                wheelQueryResults,
                vehicles_[0]->mWheelsSimData.getNbWheels()
            }
        };

        physx::PxVehicleUpdates(
            0.16f / SIM_STEPS,
            settings_.gravity,
            *surface_tire_pairs_,
            1,
            vehicles_,
            vehicleQueryResults
        );

        g_scene_->simulate(0.16f / SIM_STEPS);
        g_scene_->fetchResults(true);
    }

    // Inform rest of system of new object locations
    for (auto& object : dynamic_objects_) {
        if (!object.is_valid()) {
            continue;
        }

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

void SampleVehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData) {
    if (0 != qryFilterData->word3) {
        std::cerr << "word3 is reserved for filter data for vehicle raycast queries" << std::endl;
        assert(0 == qryFilterData->word3);
    }

    qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}

PhysicsSystem::SampleVehicleSceneQueryData* PhysicsSystem::SampleVehicleSceneQueryData::allocate(const PxU32 maxNumWheels) {
#define SIZEALIGN16(size) (((unsigned)(size)+15)&((unsigned)(~15)));

    const PxU32 size0 = SIZEALIGN16(sizeof(SampleVehicleSceneQueryData));
    const PxU32 size1 = SIZEALIGN16(sizeof(PxRaycastQueryResult) * maxNumWheels);
    const PxU32 size2 = SIZEALIGN16(sizeof(PxRaycastHit) * maxNumWheels);
    const PxU32 size = size0 + size1 + size2;
    SampleVehicleSceneQueryData* sqData = (SampleVehicleSceneQueryData*)malloc(size);
    sqData->init();
    PxU8* ptr = (PxU8*)sqData;
    ptr += size0;
    sqData->sq_results_ = (PxRaycastQueryResult*)ptr;
    sqData->nb_sq_results_ = maxNumWheels;
    ptr += size1;
    sqData->sq_hit_buffer_ = (PxRaycastHit*)ptr;
    ptr += size2;
    sqData->num_queries_ = maxNumWheels;
    return sqData;

#undef SIZEALIGN16

}

PxBatchQuery* PhysicsSystem::SampleVehicleSceneQueryData::setup_batched_scene_query(PxScene* scene) {
    PxBatchQueryDesc sqDesc(nb_sq_results_, 0, 0);
    sqDesc.queryMemory.userRaycastResultBuffer = sq_results_;
    sqDesc.queryMemory.userRaycastTouchBuffer = sq_hit_buffer_;
    sqDesc.queryMemory.raycastTouchBufferSize = num_queries_;
    sqDesc.preFilterShader = pre_filter_shader_;
    return scene->createBatchQuery(sqDesc);
}

PhysicsSystem::SampleVehicleWheelQueryResults* PhysicsSystem::SampleVehicleWheelQueryResults::allocate(const PxU32 maxNumWheels) {
    const PxU32 size = sizeof(SampleVehicleWheelQueryResults) + sizeof(PxWheelQueryResult) * maxNumWheels;
    SampleVehicleWheelQueryResults* resData = (SampleVehicleWheelQueryResults*)malloc(size);
    resData->init();
    PxU8* ptr = (PxU8*)resData;
    ptr += sizeof(SampleVehicleWheelQueryResults);
    resData->wheel_query_results_ = (PxWheelQueryResult*)ptr;
    ptr += sizeof(PxWheelQueryResult) * maxNumWheels;
    resData->max_num_wheels_ = maxNumWheels;

    for (PxU32 i = 0; i < maxNumWheels; i++) {
        new(&resData->wheel_query_results_[i]) PxWheelQueryResult();
    }

    return resData;
}
PxWheelQueryResult* PhysicsSystem::SampleVehicleWheelQueryResults::add_vehicle(const PxU32 numWheels) {
    PX_ASSERT((num_wheels_ + numWheels) <= max_num_wheels_);
    PxWheelQueryResult* r = &wheel_query_results_[num_wheels_];
    num_wheels_ += numWheels;
    return r;
}

void setupActor (
    PxRigidDynamic* vehActor,
    const PxFilterData& vehQryFilterData,
    const PxGeometry** wheelGeometries,
    const PxTransform* wheelLocalPoses,
    const PxU32 numWheelGeometries,
    const PxMaterial* wheelMaterial,
    const PxFilterData& wheelCollFilterData,
    const PxGeometry** chassisGeometries,
    const PxTransform* chassisLocalPoses,
    const PxU32 numChassisGeometries,
    const PxMaterial* chassisMaterial,
    const PxFilterData& chassisCollFilterData,
    const PxVehicleChassisData& chassisData,
    PxPhysics* physics
) {
    // Add all the wheel shapes to the actor.
    for (PxU32 i = 0; i < numWheelGeometries; i++) {
        PxShape* wheelShape = PxRigidActorExt::createExclusiveShape(*vehActor, *wheelGeometries[i], *wheelMaterial);
        wheelShape->setQueryFilterData(vehQryFilterData);
        wheelShape->setSimulationFilterData(wheelCollFilterData);
        wheelShape->setLocalPose(wheelLocalPoses[i]);
    }

    // Add the chassis shapes to the actor.
    for (PxU32 i = 0; i < numChassisGeometries; i++) {
        PxShape* chassisShape = PxRigidActorExt::createExclusiveShape(*vehActor, *chassisGeometries[i], *chassisMaterial);
        chassisShape->setQueryFilterData(vehQryFilterData);
        chassisShape->setSimulationFilterData(chassisCollFilterData);
        chassisShape->setLocalPose(chassisLocalPoses[i]);
    }

    vehActor->setMass(chassisData.mMass);
    vehActor->setMassSpaceInertiaTensor(chassisData.mMOI);
    vehActor->setCMassLocalPose(PxTransform(chassisData.mCMOffset, PxQuat(PxIdentity)));
}

PxRigidDynamic* createVehicleActor4W(
    const PxVehicleChassisData& chassisData,
    PxConvexMesh** wheelConvexMeshes,
    PxConvexMesh* chassisConvexMesh,
    PxScene& scene,
    PxPhysics& physics,
    const PxMaterial& material
) {
    // We need a rigid body actor for the vehicle.
    // Don't forget to add the actor the scene after setting up the associated vehicle.
    PxRigidDynamic* vehActor = physics.createRigidDynamic(PxTransform(PxIdentity));

    // We need to add wheel collision shapes, their local poses, a material for the wheels, and a simulation filter for the wheels.
    PxConvexMeshGeometry frontLeftWheelGeom(wheelConvexMeshes[0]);
    PxConvexMeshGeometry frontRightWheelGeom(wheelConvexMeshes[1]);
    PxConvexMeshGeometry rearLeftWheelGeom(wheelConvexMeshes[2]);
    PxConvexMeshGeometry rearRightWheelGeom(wheelConvexMeshes[3]);
    const PxGeometry* wheelGeometries[4] = { &frontLeftWheelGeom, &frontRightWheelGeom, &rearLeftWheelGeom, &rearRightWheelGeom };
    const PxTransform wheelLocalPoses[4] = { PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity) };
    const PxMaterial& wheelMaterial = material;
    PxFilterData wheelCollFilterData;
    wheelCollFilterData.word0 = COLLISION_FLAG_WHEEL;
    wheelCollFilterData.word1 = COLLISION_FLAG_WHEEL_AGAINST;
    wheelCollFilterData.word3 = SAMPLEVEHICLE_UNDRIVABLE_SURFACE;

    // We need to add chassis collision shapes, their local poses, a material for the chassis, and a simulation filter for the chassis.
    PxConvexMeshGeometry chassisConvexGeom(chassisConvexMesh);
    const PxGeometry* chassisGeoms[1] = { &chassisConvexGeom };
    const PxTransform chassisLocalPoses[1] = { PxTransform(PxIdentity) };
    const PxMaterial& chassisMaterial = material;
    PxFilterData chassisCollFilterData;
    chassisCollFilterData.word0 = COLLISION_FLAG_CHASSIS;
    chassisCollFilterData.word1 = COLLISION_FLAG_CHASSIS_AGAINST;
    chassisCollFilterData.word3 = SAMPLEVEHICLE_UNDRIVABLE_SURFACE;


    // Create a query filter data for the car to ensure that cars
    // do not attempt to drive on themselves.
    PxFilterData vehQryFilterData;
    SampleVehicleSetupVehicleShapeQueryFilterData(&vehQryFilterData);

    // Set up the physx rigid body actor with shapes, local poses, and filters.
    setupActor
    (vehActor,
     vehQryFilterData,
     wheelGeometries, wheelLocalPoses, 4, &wheelMaterial, wheelCollFilterData,
     chassisGeoms, chassisLocalPoses, 1, &chassisMaterial, chassisCollFilterData,
     chassisData,
     &physics);

    return vehActor;
}

void computeWheelWidthsAndRadii(PxConvexMesh** wheelConvexMeshes, PxF32* wheelWidths, PxF32* wheelRadii) {
    for (PxU32 i = 0; i < 4; i++) {
        const PxU32 numWheelVerts = wheelConvexMeshes[i]->getNbVertices();
        const PxVec3* wheelVerts = wheelConvexMeshes[i]->getVertices();
        PxVec3 wheelMin(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
        PxVec3 wheelMax(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);

        for (PxU32 j = 0; j < numWheelVerts; j++) {
            wheelMin.x = PxMin(wheelMin.x, wheelVerts[j].x);
            wheelMin.y = PxMin(wheelMin.y, wheelVerts[j].y);
            wheelMin.z = PxMin(wheelMin.z, wheelVerts[j].z);
            wheelMax.x = PxMax(wheelMax.x, wheelVerts[j].x);
            wheelMax.y = PxMax(wheelMax.y, wheelVerts[j].y);
            wheelMax.z = PxMax(wheelMax.z, wheelVerts[j].z);
        }

        wheelWidths[i] = wheelMax.x - wheelMin.x;
        wheelRadii[i] = PxMax(wheelMax.y, wheelMax.z) * 0.975f;
    }
}

PxVec3 computeChassisAABBDimensions(const PxConvexMesh* chassisConvexMesh) {
    const PxU32 numChassisVerts = chassisConvexMesh->getNbVertices();
    const PxVec3* chassisVerts = chassisConvexMesh->getVertices();
    PxVec3 chassisMin(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
    PxVec3 chassisMax(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);

    for (PxU32 i = 0; i < numChassisVerts; i++) {
        chassisMin.x = PxMin(chassisMin.x, chassisVerts[i].x);
        chassisMin.y = PxMin(chassisMin.y, chassisVerts[i].y);
        chassisMin.z = PxMin(chassisMin.z, chassisVerts[i].z);
        chassisMax.x = PxMax(chassisMax.x, chassisVerts[i].x);
        chassisMax.y = PxMax(chassisMax.y, chassisVerts[i].y);
        chassisMax.z = PxMax(chassisMax.z, chassisVerts[i].z);
    }

    const PxVec3 chassisDims = chassisMax - chassisMin;
    return chassisDims;
}

void createVehicle4WSimulationData(
    const PxF32 chassisMass, PxConvexMesh* chassisConvexMesh,
    const PxF32 wheelMass,
    PxConvexMesh** wheelConvexMeshes,
    const PxVec3* wheelCentreOffsets,
    PxVehicleWheelsSimData& wheelsData,
    PxVehicleDriveSimData4W& driveData,
    PxVehicleChassisData& chassisData
) {
    // Extract the chassis AABB dimensions from the chassis convex mesh.
    const PxVec3 chassisDims = computeChassisAABBDimensions(chassisConvexMesh);

    // The origin is at the center of the chassis mesh.
    // Set the center of mass to be below this point and a little towards the front.
    const PxVec3 chassisCMOffset = PxVec3(0.0f, -chassisDims.y * 0.5f + .65f, 0.25f);

    // Now compute the chassis mass and moment of inertia.
    // Use the moment of inertia of a cuboid as an approximate value for the chassis moi.
    PxVec3 chassisMOI(
        (chassisDims.y * chassisDims.y + chassisDims.z * chassisDims.z)*chassisMass / 12.0f,
        (chassisDims.x * chassisDims.x + chassisDims.z * chassisDims.z)*chassisMass / 12.0f,
        (chassisDims.x * chassisDims.x + chassisDims.y * chassisDims.y)*chassisMass / 12.0f
    );
    // A bit of tweaking here.  The car will have more responsive turning if we reduce the
    // y-component of the chassis moment of inertia.
    chassisMOI.y *= 0.8f;

    // Let's set up the chassis data structure now.
    chassisData.mMass = chassisMass;
    chassisData.mMOI = chassisMOI;
    chassisData.mCMOffset = chassisCMOffset;

    // Compute the sprung masses of each suspension spring using a helper function.
    PxF32 suspSprungMasses[4];
    PxVehicleComputeSprungMasses(4, wheelCentreOffsets, chassisCMOffset, chassisMass, 1, suspSprungMasses);


    // Extract the wheel radius and width from the wheel convex meshes.
    PxF32 wheelWidths[4];
    PxF32 wheelRadii[4];
    computeWheelWidthsAndRadii(wheelConvexMeshes, wheelWidths, wheelRadii);

    // Now compute the wheel masses and inertias components around the axle's axis.
    // http://en.wikipedia.org/wiki/List_of_moments_of_inertia
    PxF32 wheelMOIs[4];

    for (PxU32 i = 0; i < 4; i++) {
        wheelMOIs[i] = 0.5f * wheelMass * wheelRadii[i] * wheelRadii[i];
    }

    // Let's set up the wheel data structures now with radius, mass, and moi.
    PxVehicleWheelData wheels[4];

    for (PxU32 i = 0; i < 4; i++) {
        wheels[i].mRadius = wheelRadii[i] * 3.f;
        wheels[i].mMass = wheelMass;
        wheels[i].mMOI = wheelMOIs[i];
        wheels[i].mWidth = wheelWidths[i];
    }

    // Disable the handbrake from the front wheels and enable for the rear wheels
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxHandBrakeTorque = 0.0f;
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxHandBrakeTorque = 0.0f;
    wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 4000.0f;
    wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 4000.0f;
    // Enable steering for the front wheels and disable for the front wheels.
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = PxPi * 0.3333f;
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = PxPi * 0.3333f;
    wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxSteer = 0.0f;
    wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxSteer = 0.0f;

    // Let's set up the tire data structures now.
    // Put slicks on the front tires and wets on the rear tires.
    PxVehicleTireData tires[4];
    tires[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mType = TIRE_TYPE_WETS;
    tires[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mType = TIRE_TYPE_WETS;
    tires[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mType = TIRE_TYPE_WETS;
    tires[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mType = TIRE_TYPE_WETS;

    // Let's set up the suspension data structures now.
    PxVehicleSuspensionData susps[4];

    for (PxU32 i = 0; i < 4; i++) {
        susps[i].mMaxCompression = 0.3f;
        susps[i].mMaxDroop = 0.1f;
        susps[i].mSpringStrength = 35000.0f;
        susps[i].mSpringDamperRate = 4500.0f;
    }

    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eFRONT_LEFT];
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT];
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eREAR_LEFT];
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eREAR_RIGHT];

    // Set up the camber.
    // Remember that the left and right wheels need opposite camber so that the car preserves symmetry about the forward direction.
    // Set the camber to 0.0f when the spring is neither compressed or elongated.
    const PxF32 camberAngleAtRest = 0.0;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtRest = camberAngleAtRest;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtRest = -camberAngleAtRest;
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtRest = camberAngleAtRest;
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtRest = -camberAngleAtRest;
    // Set the wheels to camber inwards at maximum droop (the left and right wheels almost form a V shape)
    const PxF32 camberAngleAtMaxDroop = 0.001f;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxDroop = camberAngleAtMaxDroop;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxDroop = camberAngleAtMaxDroop;
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
    // Set the wheels to camber outwards at maximum compression (the left and right wheels almost form a A shape).
    const PxF32 camberAngleAtMaxCompression = -0.001f;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxCompression = camberAngleAtMaxCompression;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxCompression = -camberAngleAtMaxCompression;
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxCompression = camberAngleAtMaxCompression;
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxCompression = -camberAngleAtMaxCompression;

    // We need to set up geometry data for the suspension, wheels, and tires.
    // We already know the wheel centers described as offsets from the actor center and the center of mass offset from actor center.
    // From here we can approximate application points for the tire and suspension forces.
    // Lets assume that the suspension travel directions are absolutely vertical.
    // Also assume that we apply the tire and suspension forces 30cm below the center of mass.
    PxVec3 suspTravelDirections[4] = { PxVec3(0, -1, 0), PxVec3(0, -1, 0), PxVec3(0, -1, 0), PxVec3(0, -1, 0) };
    PxVec3 wheelCentreCMOffsets[4];
    PxVec3 suspForceAppCMOffsets[4];
    PxVec3 tireForceAppCMOffsets[4];

    for (PxU32 i = 0; i < 4; i++) {
        wheelCentreCMOffsets[i] = wheelCentreOffsets[i] - chassisCMOffset;
        suspForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, 1.3f, wheelCentreCMOffsets[i].z);
        tireForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, 1.3f, wheelCentreCMOffsets[i].z);
    }

    // Now add the wheel, tire and suspension data.
    for (PxU32 i = 0; i < 4; i++) {
        wheelsData.setWheelData(i, wheels[i]);
        wheelsData.setTireData(i, tires[i]);
        wheelsData.setSuspensionData(i, susps[i]);
        wheelsData.setSuspTravelDirection(i, suspTravelDirections[i]);
        wheelsData.setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
        wheelsData.setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
        wheelsData.setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
    }

    // Set the car to perform 3 sub-steps when it moves with a forwards speed of less than 5.0
    // and with a single step when it moves at speed greater than or equal to 5.0.
    wheelsData.setSubStepCount(5.0f, 3, 1);

    // Now set up the differential, engine, gears, clutch, and ackermann steering.

    // Diff
    PxVehicleDifferential4WData diff;
    diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
    driveData.setDiffData(diff);

    // Engine
    PxVehicleEngineData engine;
    engine.mPeakTorque = 50.0f;
    engine.mMaxOmega = 60.0f; // approx 6000 rpm
    driveData.setEngineData(engine);

    // Gears
    PxVehicleGearsData gears;
    gears.mSwitchTime = 0.5f;
    driveData.setGearsData(gears);

    // Clutch
    PxVehicleClutchData clutch;
    clutch.mStrength = 10.0f;
    driveData.setClutchData(clutch);

    // Ackermann steer accuracy
    PxVehicleAckermannGeometryData ackermann;
    ackermann.mAccuracy = 1.0f;
    ackermann.mAxleSeparation = wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].z - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].z;
    ackermann.mFrontWidth = wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].x - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].x;
    ackermann.mRearWidth = wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].x - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].x;
    driveData.setAckermannGeometryData(ackermann);
}

void PhysicsSystem::create_4w_vehicle (
    PxScene& scene,
    PxPhysics& physics,
    PxCooking& cooking,
    const PxMaterial& material,
    const PxF32 chassisMass,
    const PxVec3* wheelCentreOffsets4,
    PxConvexMesh* chassisConvexMesh,
    PxConvexMesh** wheelConvexMeshes4,
    const PxTransform& startTransform,
    const bool useAutoGearFlag
) {
    PX_ASSERT(num_vehicles_ < MAX_NUM_4W_VEHICLES);

    PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(4);
    PxVehicleDriveSimData4W driveSimData;
    PxVehicleChassisData chassisData;
    createVehicle4WSimulationData
    (chassisMass, chassisConvexMesh,
     20.0f, wheelConvexMeshes4, wheelCentreOffsets4,
     *wheelsSimData, driveSimData, chassisData);

    // Instantiate and finalize the vehicle using physx.
    PxRigidDynamic* vehActor = createVehicleActor4W(chassisData, wheelConvexMeshes4, chassisConvexMesh, scene, physics, material);

    // Create a car.
    PxVehicleDrive4W* car = PxVehicleDrive4W::allocate(4);
    car->setup(&physics, vehActor, *wheelsSimData, driveSimData, 0);

    // Free the sim data because we don't need that any more.
    wheelsSimData->free();

    // Don't forget to add the actor to the scene.
    {
        PxSceneWriteLock scopedLock(scene);
        scene.addActor(*vehActor);
    }

    // Set up the mapping between wheel and actor shape.
    car->mWheelsSimData.setWheelShapeMapping(0, 0);
    car->mWheelsSimData.setWheelShapeMapping(1, 1);
    car->mWheelsSimData.setWheelShapeMapping(2, 2);
    car->mWheelsSimData.setWheelShapeMapping(3, 3);

    // Set up the scene query filter data for each suspension line.
    PxFilterData vehQryFilterData;
    SampleVehicleSetupVehicleShapeQueryFilterData(&vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(0, vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(1, vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(2, vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(3, vehQryFilterData);

    // Set the transform and the instantiated car and set it be to be at rest.
    // resetNWCar(startTransform, car);
    // Set the autogear mode of the instantiate car.
    car->mDriveDynData.setUseAutoGears(useAutoGearFlag);

    // Increment the number of vehicles
    vehicles_[num_vehicles_] = car;
    vehicle_wheel_query_results_[num_vehicles_].nbWheelQueryResults = 4;
    vehicle_wheel_query_results_[num_vehicles_].wheelQueryResults = wheel_query_results->add_vehicle(4);
    num_vehicles_++;

    drive_sim_data_4w_ = driveSimData;
}

void PhysicsSystem::handle_key_press(const Event& e) {
    int player_id = e.get_value<int>("player_id", true).first;
    int key = e.get_value<int>("key", true).first;
    int value = e.get_value<int>("value", true).first;

    switch (key) {
        case SDLK_w:
            forward_drive_ = 0.5f;
            backward_drive_ = 0.0f;
            break;

        case SDLK_s:
            forward_drive_ = 0.0f;
            backward_drive_ = 0.5f;
            break;

        case SDLK_a:
            horizontal_drive_ = 0.5f;
            break;

        case SDLK_d:
            horizontal_drive_ = -0.5f;
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            forward_drive_ = (float)value / 32768;
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            backward_drive_ = (float)value / 32768;
            break;

        case SDL_CONTROLLER_AXIS_LEFTX:
            horizontal_drive_ = (float)value / -32768;
            break;

        case SDL_CONTROLLER_BUTTON_B:
            hand_break_ = true;

        default:
            break;
    }
}

PxConvexMesh* createConvexMesh(const PxVec3* verts, const PxU32 numVerts, PxPhysics& physics, PxCooking& cooking) {
    // Create descriptor for convex mesh
    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = numVerts;
    convexDesc.points.stride = sizeof(PxVec3);
    convexDesc.points.data = verts;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxConvexMesh* convexMesh = NULL;
    PxDefaultMemoryOutputStream buf;

    if (cooking.cookConvexMesh(convexDesc, buf)) {
        PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
        convexMesh = physics.createConvexMesh(id);
    }

    return convexMesh;
}

PxConvexMesh* createCylinderConvexMesh(const PxF32 width, const PxF32 radius, const PxU32 numCirclePoints, PxPhysics& physics, PxCooking& cooking) {
#define  MAX_NUM_VERTS_IN_CIRCLE 16
    PX_ASSERT(numCirclePoints < MAX_NUM_VERTS_IN_CIRCLE);
    PxVec3 verts[2 * MAX_NUM_VERTS_IN_CIRCLE];
    PxU32 numVerts = 2 * numCirclePoints;
    const PxF32 dtheta = 2 * PxPi / (1.0f * numCirclePoints);

    for (PxU32 i = 0; i < MAX_NUM_VERTS_IN_CIRCLE; i++) {
        const PxF32 theta = dtheta * i;
        const PxF32 cosTheta = radius * PxCos(theta);
        const PxF32 sinTheta = radius * PxSin(theta);
        verts[2 * i + 0] = PxVec3(-0.5f * width, cosTheta, sinTheta);
        verts[2 * i + 1] = PxVec3(+0.5f * width, cosTheta, sinTheta);
    }

    return createConvexMesh(verts, numVerts, physics, cooking);

#undef MAX_NUM_VERTS_IN_CIRCLE

}

PxConvexMesh* createWheelConvexMesh(const PxVec3* verts, const PxU32 numVerts, PxPhysics& physics, PxCooking& cooking) {
    //Extract the wheel radius and width from the aabb of the wheel convex mesh.
    PxVec3 wheelMin(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
    PxVec3 wheelMax(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);

    for (PxU32 i = 0; i < numVerts; i++) {
        wheelMin.x = PxMin(wheelMin.x, verts[i].x);
        wheelMin.y = PxMin(wheelMin.y, verts[i].y);
        wheelMin.z = PxMin(wheelMin.z, verts[i].z);
        wheelMax.x = PxMax(wheelMax.x, verts[i].x);
        wheelMax.y = PxMax(wheelMax.y, verts[i].y);
        wheelMax.z = PxMax(wheelMax.z, verts[i].z);
    }

    const PxF32 wheelWidth = wheelMax.x - wheelMin.x;
    const PxF32 wheelRadius = PxMax(wheelMax.y, wheelMax.z);

    return createCylinderConvexMesh(wheelWidth, wheelRadius, 8, physics, cooking);
}

void PhysicsSystem::handle_add_example_ship(const Event& e) {
    if (num_vehicles_) {
        return;
    }

    int object_id = e.get_value<int>("object_id", true).first;
    physx::PxTransform transform(0.f, 0.f, 0.f);

    transform.p.x = e.get_value<int>("pos_x", true).first;
    transform.p.y = e.get_value<int>("pos_y", true).first;
    transform.p.z = e.get_value<int>("pos_z", true).first;
    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/carBoxModel.obj");

    dynamic_objects_.emplace_back(object_id);
    auto& vehicle = dynamic_objects_.back();
    vehicle.set_mesh(g_physics_, g_cooking_, mesh);
    auto mat = vehicle.get_material();
    auto mesh_mesh = vehicle.get_mesh();


    float s = 0.2f;
    PxVec3 verts[8] = {
        {s, s, s},
        { -s, s, s},
        { s, s, -s },
        { -s, s, -s },
        { s, -s, s },
        { -s, -s, s },
        { s, -s, -s },
        { -s, -s, -s }
    };

    PxConvexMesh* wheel_mesh[4] = {
        createWheelConvexMesh(verts, 8, *g_physics_, *g_cooking_),
        createWheelConvexMesh(verts, 8, *g_physics_, *g_cooking_),
        createWheelConvexMesh(verts, 8, *g_physics_, *g_cooking_),
        createWheelConvexMesh(verts, 8, *g_physics_, *g_cooking_)
    };

    PxVec3 wheelCenterOffsets[4];
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT] = physx::PxVec3(-1.5f, 0.5f, 1.f);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] = physx::PxVec3(1.5f, 0.5f, 1.f);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT] = physx::PxVec3(-1.5f, 0.5f, -1.f);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT] = physx::PxVec3(1.5f, 0.5f, -1.f);

    create_4w_vehicle(*g_scene_, *g_physics_, *g_cooking_, *mat, 1500.f, wheelCenterOffsets, mesh_mesh, wheel_mesh, transform, true);
    vehicle.set_actor(vehicles_[num_vehicles_ - 1]->getRigidDynamicActor());
    vehicle.set_transform(transform);
}

PxVehicleDrivableSurfaceToTireFrictionPairs* createFrictionPairs(const PxMaterial* defaultMaterial) {
    PxVehicleDrivableSurfaceType surfaceTypes[1];
    surfaceTypes[0].mType = 0;

    const PxMaterial* surfaceMaterials[1];
    surfaceMaterials[0] = defaultMaterial;

    PxVehicleDrivableSurfaceToTireFrictionPairs* surfaceTirePairs =
        PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(1, 1);

    surfaceTirePairs->setup(1, 1, surfaceMaterials, surfaceTypes);

    for (PxU32 i = 0; i < 1; i++) {
        for (PxU32 j = 0; j < 1; j++) {
            surfaceTirePairs->setTypePairFriction(i, j, 1.f);
        }
    }

    return surfaceTirePairs;
}

void PhysicsSystem::handle_add_terrain(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/Terrain.obj");

    static_objects_.emplace_back(object_id);
    static_objects_.back().set_mesh(g_physics_, g_cooking_, mesh);

    surface_tire_pairs_ = createFrictionPairs(static_objects_.back().get_material());

    g_scene_->addActor(*static_objects_.back().get_actor());
}
