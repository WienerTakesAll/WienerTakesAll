#include "PhysicsSystem.h"

#include "AssetManager.h"
#include "SDL.h"

#include "vehicle/PxVehicleTireFriction.h"
#include "vehicle/PxVehicleWheels.h"

#include <assert.h>
#include <glm/gtx/quaternion.hpp>


#include "MeshStuff.h"

const physx::PxVec3 GRAVITY(0.0f, 0.1f*-9.81f, 0.0f);


/*
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
*/


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








using namespace physx;

void SampleVehicleSetupDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
    assert(0 == qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
    qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_DRIVABLE_SURFACE;
}

void SampleVehicleSetupNonDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
    assert(0 == qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
    qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}

void SampleVehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData)
{
    assert(0 == qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
    qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}

PhysicsSystem::SampleVehicleSceneQueryData* PhysicsSystem::SampleVehicleSceneQueryData::allocate(const PxU32 maxNumWheels)
{
#define SIZEALIGN16(size) (((unsigned)(size)+15)&((unsigned)(~15)));


    const PxU32 size0 = SIZEALIGN16(sizeof(SampleVehicleSceneQueryData));
    const PxU32 size1 = SIZEALIGN16(sizeof(PxRaycastQueryResult)*maxNumWheels);
    const PxU32 size2 = SIZEALIGN16(sizeof(PxRaycastHit)*maxNumWheels);
    const PxU32 size = size0 + size1 + size2;
    SampleVehicleSceneQueryData* sqData = (SampleVehicleSceneQueryData*)malloc(size);
    sqData->init();
    PxU8* ptr = (PxU8*)sqData;
    ptr += size0;
    sqData->mSqResults = (PxRaycastQueryResult*)ptr;
    sqData->mNbSqResults = maxNumWheels;
    ptr += size1;
    sqData->mSqHitBuffer = (PxRaycastHit*)ptr;
    ptr += size2;
    sqData->mNumQueries = maxNumWheels;
    return sqData;
}

void PhysicsSystem::SampleVehicleSceneQueryData::free()
{
    //PX_FREE(this);
}

PxBatchQuery* PhysicsSystem::SampleVehicleSceneQueryData::setUpBatchedSceneQuery(PxScene* scene)
{
    PxBatchQueryDesc sqDesc(mNbSqResults, 0, 0);
    sqDesc.queryMemory.userRaycastResultBuffer = mSqResults;
    sqDesc.queryMemory.userRaycastTouchBuffer = mSqHitBuffer;
    sqDesc.queryMemory.raycastTouchBufferSize = mNumQueries;
    sqDesc.preFilterShader = mPreFilterShader;
    return scene->createBatchQuery(sqDesc);
}


















PhysicsSystem::SampleVehicleWheelQueryResults* PhysicsSystem::SampleVehicleWheelQueryResults::allocate(const PxU32 maxNumWheels)
{
    const PxU32 size = sizeof(SampleVehicleWheelQueryResults) + sizeof(PxWheelQueryResult)*maxNumWheels;
    SampleVehicleWheelQueryResults* resData = (SampleVehicleWheelQueryResults*)malloc(size);
    resData->init();
    PxU8* ptr = (PxU8*)resData;
    ptr += sizeof(SampleVehicleWheelQueryResults);
    resData->mWheelQueryResults = (PxWheelQueryResult*)ptr;
    ptr += sizeof(PxWheelQueryResult)*maxNumWheels;
    resData->mMaxNumWheels = maxNumWheels;
    for (PxU32 i = 0; i<maxNumWheels; i++)
    {
        new(&resData->mWheelQueryResults[i]) PxWheelQueryResult();
    }
    return resData;
}

void PhysicsSystem::SampleVehicleWheelQueryResults::free()
{
    //PX_FREE(this);
}

PxWheelQueryResult* PhysicsSystem::SampleVehicleWheelQueryResults::addVehicle(const PxU32 numWheels)
{
    PX_ASSERT((mNumWheels + numWheels) <= mMaxNumWheels);
    PxWheelQueryResult* r = &mWheelQueryResults[mNumWheels];
    mNumWheels += numWheels;
    return r;
}


/*
void SampleVehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData)
{
    assert(0 == qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
    qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}
*/


void setupActor
(PxRigidDynamic* vehActor,
    const PxFilterData& vehQryFilterData,
    const PxGeometry** wheelGeometries, const PxTransform* wheelLocalPoses, const PxU32 numWheelGeometries, const PxMaterial* wheelMaterial, const PxFilterData& wheelCollFilterData,
    const PxGeometry** chassisGeometries, const PxTransform* chassisLocalPoses, const PxU32 numChassisGeometries, const PxMaterial* chassisMaterial, const PxFilterData& chassisCollFilterData,
    const PxVehicleChassisData& chassisData,
    PxPhysics* physics)
{
    //Add all the wheel shapes to the actor.
    for (PxU32 i = 0; i<numWheelGeometries; i++)
    {
        PxShape* wheelShape = PxRigidActorExt::createExclusiveShape(*vehActor, *wheelGeometries[i], *wheelMaterial);
        wheelShape->setQueryFilterData(vehQryFilterData);
        wheelShape->setSimulationFilterData(wheelCollFilterData);
        wheelShape->setLocalPose(wheelLocalPoses[i]);
    }

    //Add the chassis shapes to the actor.
    for (PxU32 i = 0; i<numChassisGeometries; i++)
    {
        PxShape* chassisShape = PxRigidActorExt::createExclusiveShape(*vehActor, *chassisGeometries[i], *chassisMaterial);
        chassisShape->setQueryFilterData(vehQryFilterData);
        chassisShape->setSimulationFilterData(chassisCollFilterData);
        chassisShape->setLocalPose(chassisLocalPoses[i]);
    }

    vehActor->setMass(chassisData.mMass);
    vehActor->setMassSpaceInertiaTensor(chassisData.mMOI);
    vehActor->setCMassLocalPose(PxTransform(chassisData.mCMOffset, PxQuat(PxIdentity)));
}

PxRigidDynamic* createVehicleActor4W
(const PxVehicleChassisData& chassisData,
    PxConvexMesh** wheelConvexMeshes, PxConvexMesh* chassisConvexMesh,
    PxScene& scene, PxPhysics& physics, const PxMaterial& material)
{
    //We need a rigid body actor for the vehicle.
    //Don't forget to add the actor the scene after setting up the associated vehicle.
    PxRigidDynamic* vehActor = physics.createRigidDynamic(PxTransform(PxIdentity));

    //We need to add wheel collision shapes, their local poses, a material for the wheels, and a simulation filter for the wheels.
    PxConvexMeshGeometry frontLeftWheelGeom(wheelConvexMeshes[0]);
    PxConvexMeshGeometry frontRightWheelGeom(wheelConvexMeshes[1]);
    PxConvexMeshGeometry rearLeftWheelGeom(wheelConvexMeshes[2]);
    PxConvexMeshGeometry rearRightWheelGeom(wheelConvexMeshes[3]);
    const PxGeometry* wheelGeometries[4] = { &frontLeftWheelGeom,&frontRightWheelGeom,&rearLeftWheelGeom,&rearRightWheelGeom };
    const PxTransform wheelLocalPoses[4] = { PxTransform(PxIdentity),PxTransform(PxIdentity),PxTransform(PxIdentity),PxTransform(PxIdentity) };
    const PxMaterial& wheelMaterial = material;
    PxFilterData wheelCollFilterData;
    wheelCollFilterData.word0 = COLLISION_FLAG_WHEEL;
    wheelCollFilterData.word1 = COLLISION_FLAG_WHEEL_AGAINST;
    wheelCollFilterData.word3 = SAMPLEVEHICLE_UNDRIVABLE_SURFACE;

    //We need to add chassis collision shapes, their local poses, a material for the chassis, and a simulation filter for the chassis.
    PxConvexMeshGeometry chassisConvexGeom(chassisConvexMesh);
    const PxGeometry* chassisGeoms[1] = { &chassisConvexGeom };
    const PxTransform chassisLocalPoses[1] = { PxTransform(PxIdentity) };
    const PxMaterial& chassisMaterial = material;
    PxFilterData chassisCollFilterData;
    chassisCollFilterData.word0 = COLLISION_FLAG_CHASSIS;
    chassisCollFilterData.word1 = COLLISION_FLAG_CHASSIS_AGAINST;
    chassisCollFilterData.word3 = SAMPLEVEHICLE_UNDRIVABLE_SURFACE;


    //Create a query filter data for the car to ensure that cars
    //do not attempt to drive on themselves.
    PxFilterData vehQryFilterData;
    SampleVehicleSetupVehicleShapeQueryFilterData(&vehQryFilterData);

    //Set up the physx rigid body actor with shapes, local poses, and filters.
    setupActor
    (vehActor,
        vehQryFilterData,
        wheelGeometries, wheelLocalPoses, 4, &wheelMaterial, wheelCollFilterData,
        chassisGeoms, chassisLocalPoses, 1, &chassisMaterial, chassisCollFilterData,
        chassisData,
        &physics);

    return vehActor;
}

void computeWheelWidthsAndRadii(PxConvexMesh** wheelConvexMeshes, PxF32* wheelWidths, PxF32* wheelRadii)
{
    for (PxU32 i = 0; i<4; i++)
    {
        const PxU32 numWheelVerts = wheelConvexMeshes[i]->getNbVertices();
        const PxVec3* wheelVerts = wheelConvexMeshes[i]->getVertices();
        PxVec3 wheelMin(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
        PxVec3 wheelMax(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);
        for (PxU32 j = 0; j<numWheelVerts; j++)
        {
            wheelMin.x = PxMin(wheelMin.x, wheelVerts[j].x);
            wheelMin.y = PxMin(wheelMin.y, wheelVerts[j].y);
            wheelMin.z = PxMin(wheelMin.z, wheelVerts[j].z);
            wheelMax.x = PxMax(wheelMax.x, wheelVerts[j].x);
            wheelMax.y = PxMax(wheelMax.y, wheelVerts[j].y);
            wheelMax.z = PxMax(wheelMax.z, wheelVerts[j].z);
        }
        wheelWidths[i] = wheelMax.x - wheelMin.x;
        wheelRadii[i] = PxMax(wheelMax.y, wheelMax.z)*0.975f;
    }
}

PxVec3 computeChassisAABBDimensions(const PxConvexMesh* chassisConvexMesh)
{
    const PxU32 numChassisVerts = chassisConvexMesh->getNbVertices();
    const PxVec3* chassisVerts = chassisConvexMesh->getVertices();
    PxVec3 chassisMin(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
    PxVec3 chassisMax(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);
    for (PxU32 i = 0; i<numChassisVerts; i++)
    {
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

void createVehicle4WSimulationData
(const PxF32 chassisMass, PxConvexMesh* chassisConvexMesh,
    const PxF32 wheelMass, PxConvexMesh** wheelConvexMeshes, const PxVec3* wheelCentreOffsets,
    PxVehicleWheelsSimData& wheelsData, PxVehicleDriveSimData4W& driveData, PxVehicleChassisData& chassisData)
{
    //Extract the chassis AABB dimensions from the chassis convex mesh.
    const PxVec3 chassisDims = computeChassisAABBDimensions(chassisConvexMesh);

    //The origin is at the center of the chassis mesh.
    //Set the center of mass to be below this point and a little towards the front.
    const PxVec3 chassisCMOffset = PxVec3(0.0f, -chassisDims.y*0.5f + .65f, 0.25f);

    //Now compute the chassis mass and moment of inertia.
    //Use the moment of inertia of a cuboid as an approximate value for the chassis moi.
    PxVec3 chassisMOI
    ((chassisDims.y*chassisDims.y + chassisDims.z*chassisDims.z)*chassisMass / 12.0f,
        (chassisDims.x*chassisDims.x + chassisDims.z*chassisDims.z)*chassisMass / 12.0f,
        (chassisDims.x*chassisDims.x + chassisDims.y*chassisDims.y)*chassisMass / 12.0f);
    //A bit of tweaking here.  The car will have more responsive turning if we reduce the 	
    //y-component of the chassis moment of inertia.
    chassisMOI.y *= 0.8f;

    //Let's set up the chassis data structure now.
    chassisData.mMass = chassisMass;
    chassisData.mMOI = chassisMOI;
    chassisData.mCMOffset = chassisCMOffset;

    //Compute the sprung masses of each suspension spring using a helper function.
    PxF32 suspSprungMasses[4];
    PxVehicleComputeSprungMasses(4, wheelCentreOffsets, chassisCMOffset, chassisMass, 1, suspSprungMasses);



    //Extract the wheel radius and width from the wheel convex meshes.
    PxF32 wheelWidths[4];
    PxF32 wheelRadii[4];
    computeWheelWidthsAndRadii(wheelConvexMeshes, wheelWidths, wheelRadii);

    //Now compute the wheel masses and inertias components around the axle's axis.
    //http://en.wikipedia.org/wiki/List_of_moments_of_inertia
    PxF32 wheelMOIs[4];
    for (PxU32 i = 0; i<4; i++)
    {
        wheelMOIs[i] = 0.5f*wheelMass*wheelRadii[i] * wheelRadii[i];
    }
    //Let's set up the wheel data structures now with radius, mass, and moi.
    PxVehicleWheelData wheels[4];
    for (PxU32 i = 0; i<4; i++)
    {
        wheels[i].mRadius = wheelRadii[i] * 3.f;
        wheels[i].mMass = wheelMass;
        wheels[i].mMOI = wheelMOIs[i];
        wheels[i].mWidth = wheelWidths[i];
    }
    //Disable the handbrake from the front wheels and enable for the rear wheels
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxHandBrakeTorque = 0.0f;
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxHandBrakeTorque = 0.0f;
    wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 4000.0f;
    wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 4000.0f;
    //Enable steering for the front wheels and disable for the front wheels.
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = PxPi*0.3333f;
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = PxPi*0.3333f;
    wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxSteer = 0.0f;
    wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxSteer = 0.0f;

    //Let's set up the tire data structures now.
    //Put slicks on the front tires and wets on the rear tires.
    PxVehicleTireData tires[4];
    tires[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mType = TIRE_TYPE_WETS;
    tires[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mType = TIRE_TYPE_WETS;
    tires[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mType = TIRE_TYPE_WETS;
    tires[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mType = TIRE_TYPE_WETS;

    //Let's set up the suspension data structures now.
    PxVehicleSuspensionData susps[4];
    for (PxU32 i = 0; i<4; i++)
    {
        susps[i].mMaxCompression = 0.3f;
        susps[i].mMaxDroop = 0.1f;
        susps[i].mSpringStrength = 35000.0f;
        susps[i].mSpringDamperRate = 4500.0f;
    }
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eFRONT_LEFT];
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT];
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eREAR_LEFT];
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eREAR_RIGHT];

    //Set up the camber.
    //Remember that the left and right wheels need opposite camber so that the car preserves symmetry about the forward direction.
    //Set the camber to 0.0f when the spring is neither compressed or elongated.
    const PxF32 camberAngleAtRest = 0.0;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtRest = camberAngleAtRest;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtRest = -camberAngleAtRest;
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtRest = camberAngleAtRest;
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtRest = -camberAngleAtRest;
    //Set the wheels to camber inwards at maximum droop (the left and right wheels almost form a V shape)
    const PxF32 camberAngleAtMaxDroop = 0.001f;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxDroop = camberAngleAtMaxDroop;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxDroop = camberAngleAtMaxDroop;
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
    //Set the wheels to camber outwards at maximum compression (the left and right wheels almost form a A shape).
    const PxF32 camberAngleAtMaxCompression = -0.001f;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxCompression = camberAngleAtMaxCompression;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxCompression = -camberAngleAtMaxCompression;
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxCompression = camberAngleAtMaxCompression;
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxCompression = -camberAngleAtMaxCompression;

    //We need to set up geometry data for the suspension, wheels, and tires.
    //We already know the wheel centers described as offsets from the actor center and the center of mass offset from actor center.
    //From here we can approximate application points for the tire and suspension forces.
    //Lets assume that the suspension travel directions are absolutely vertical.
    //Also assume that we apply the tire and suspension forces 30cm below the center of mass.
    PxVec3 suspTravelDirections[4] = { PxVec3(0,-1,0),PxVec3(0,-1,0),PxVec3(0,-1,0),PxVec3(0,-1,0) };
    PxVec3 wheelCentreCMOffsets[4];
    PxVec3 suspForceAppCMOffsets[4];
    PxVec3 tireForceAppCMOffsets[4];
    for (PxU32 i = 0; i<4; i++)
    {
        wheelCentreCMOffsets[i] = wheelCentreOffsets[i] - chassisCMOffset;
        suspForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, 1.3f, wheelCentreCMOffsets[i].z);
        tireForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, 1.3f, wheelCentreCMOffsets[i].z);
    }

    //Now add the wheel, tire and suspension data.
    for (PxU32 i = 0; i<4; i++)
    {
        wheelsData.setWheelData(i, wheels[i]);
        wheelsData.setTireData(i, tires[i]);
        wheelsData.setSuspensionData(i, susps[i]);
        wheelsData.setSuspTravelDirection(i, suspTravelDirections[i]);
        wheelsData.setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
        wheelsData.setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
        wheelsData.setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
    }

    //Set the car to perform 3 sub-steps when it moves with a forwards speed of less than 5.0 
    //and with a single step when it moves at speed greater than or equal to 5.0.
    wheelsData.setSubStepCount(5.0f, 3, 1);


    //Now set up the differential, engine, gears, clutch, and ackermann steering.

    //Diff
    PxVehicleDifferential4WData diff;
    diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
    driveData.setDiffData(diff);

    //Engine
    PxVehicleEngineData engine;
    engine.mPeakTorque = 50.0f;
    engine.mMaxOmega = 60.0f;//approx 6000 rpm
    driveData.setEngineData(engine);

    //Gears
    PxVehicleGearsData gears;
    gears.mSwitchTime = 0.5f;
    driveData.setGearsData(gears);
 

    //Clutch
    PxVehicleClutchData clutch;
    clutch.mStrength = 10.0f;
    driveData.setClutchData(clutch);

    //Ackermann steer accuracy
    PxVehicleAckermannGeometryData ackermann;
    ackermann.mAccuracy = 1.0f;
    ackermann.mAxleSeparation = wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].z - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].z;
    ackermann.mFrontWidth = wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].x - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].x;
    ackermann.mRearWidth = wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].x - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].x;
    driveData.setAckermannGeometryData(ackermann);
}


void PhysicsSystem::create4WVehicle
(PxScene& scene, PxPhysics& physics, PxCooking& cooking, const PxMaterial& material,
    const PxF32 chassisMass, const PxVec3* wheelCentreOffsets4, PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes4,
    const PxTransform& startTransform, const bool useAutoGearFlag)
{
    PX_ASSERT(mNumVehicles<MAX_NUM_4W_VEHICLES);

    PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(4);
    PxVehicleDriveSimData4W driveSimData;
    PxVehicleChassisData chassisData;
    createVehicle4WSimulationData
    (chassisMass, chassisConvexMesh,
        20.0f, wheelConvexMeshes4, wheelCentreOffsets4,
        *wheelsSimData, driveSimData, chassisData);

    //Instantiate and finalize the vehicle using physx.
    PxRigidDynamic* vehActor = createVehicleActor4W(chassisData, wheelConvexMeshes4, chassisConvexMesh, scene, physics, material);

    //Create a car.
    PxVehicleDrive4W* car = PxVehicleDrive4W::allocate(4);
    car->setup(&physics, vehActor, *wheelsSimData, driveSimData, 0);




    //Free the sim data because we don't need that any more.
    wheelsSimData->free();

    //Don't forget to add the actor to the scene.
    {
        PxSceneWriteLock scopedLock(scene);
        scene.addActor(*vehActor);
    }


    //Set up the mapping between wheel and actor shape.
    car->mWheelsSimData.setWheelShapeMapping(0, 0);
    car->mWheelsSimData.setWheelShapeMapping(1, 1);
    car->mWheelsSimData.setWheelShapeMapping(2, 2);
    car->mWheelsSimData.setWheelShapeMapping(3, 3);

    //Set up the scene query filter data for each suspension line.
    PxFilterData vehQryFilterData;
    SampleVehicleSetupVehicleShapeQueryFilterData(&vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(0, vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(1, vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(2, vehQryFilterData);
    car->mWheelsSimData.setSceneQueryFilterData(3, vehQryFilterData);

    //Set the transform and the instantiated car and set it be to be at rest.
    //resetNWCar(startTransform, car);
    //Set the autogear mode of the instantiate car.
    car->mDriveDynData.setUseAutoGears(useAutoGearFlag);

    //Increment the number of vehicles
    mVehicles[mNumVehicles] = car;
    mVehicleWheelQueryResults[mNumVehicles].nbWheelQueryResults = 4;
    mVehicleWheelQueryResults[mNumVehicles].wheelQueryResults = mWheelQueryResults->addVehicle(4);
    mNumVehicles++;


    mDriveSimData4W = driveSimData;
}

PhysicsSystem::PhysicsSystem(AssetManager& asset_manager)
    : gFoundation_(PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator_, gErrorCallback_))
    , gScale_()
    , gPvd_(PxCreatePvd(*gFoundation_))
    , gPhysics_(PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation_, gScale_, false, gPvd_))
    , gCooking_(PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation_, gScale_))
    , gScene_(nullptr)
    , mSqWheelRaycastBatchQuery(NULL)
    , asset_manager_(asset_manager) 
    , forwardDrive(0.0f), horizontalDrive(0.0f)
    , mNumVehicles(0) {
    EventSystem::add_event_handler(EventType::ADD_EXAMPLE_SHIP_EVENT, &PhysicsSystem::handle_add_example_ship, this);
    EventSystem::add_event_handler(EventType::ADD_TERRAIN_EVENT, &PhysicsSystem::handle_add_terrain, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &PhysicsSystem::handle_key_press, this);


    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    gPvd_->connect(*transport, PxPvdInstrumentationFlag::eALL);

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



    //const PxMaterial* drivableSurfaceMaterials[MAX_NUM_SURFACE_TYPES] = { { gPhysics_->createMaterial(2.f,2.f,2.f) } };
    //PxVehicleDrivableSurfaceType drivableSurfaceTypes[MAX_NUM_SURFACE_TYPES];


    //Set the vehicle update mode to be immediate velocity changes.
    PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);

    //Initialise all vehicle ptrs to null.
    for (PxU32 i = 0; i<MAX_NUM_4W_VEHICLES; i++)
    {
        mVehicles[i] = NULL;
    }

    //Allocate simulation data so we can switch from 3-wheeled to 4-wheeled cars by switching simulation data.
    mWheelsSimData4W = PxVehicleWheelsSimData::allocate(4);

    //Scene query data for to allow raycasts for all suspensions of all vehicles.
    mSqData = SampleVehicleSceneQueryData::allocate(MAX_NUM_4W_VEHICLES * 4);

    //Data to store reports for each wheel.
    mWheelQueryResults = SampleVehicleWheelQueryResults::allocate(MAX_NUM_4W_VEHICLES*4);







}

PhysicsSystem::~PhysicsSystem() {
    physx::PxCloseVehicleSDK();

    gCooking_->release();
    gPhysics_->release();
    gPvd_->release();
    gFoundation_->release();
}


float gearedUp = 5.f;

void PhysicsSystem::update()
{
    if (!gScene_)
        return;
    
    using namespace physx;

    std::vector<physx::PxVehicleWheels*> wheels;

    for (auto& object : dynamic_objects_) {
        auto pose = object.get_actor()->getGlobalPose();
        auto rotate = glm::toMat4(glm::quat(pose.q.w,pose.q.x,pose.q.y,pose.q.z));

        //object.get_actor()->addForce(physx::PxVec3(rotate[2][2]*forwardDrive, 0.f, rotate[2][0]*forwardDrive));
        //object.get_actor()->addTorque(physx::PxVec3(0.f, horizontalDrive*0.1f, 0.f));

        if (object.get_wheels()) {
            wheels.push_back(object.get_wheels());
            break;
        }
    }



    int SIM_STEPS = 4;
    
    for (int i = 0; i < SIM_STEPS; i++)
    {
        if (mNumVehicles) {


            physx::PxVehicleDrive4WRawInputData gVehicleInputData;
            gVehicleInputData.setDigitalAccel(true);

            gVehicleInputData.setAnalogAccel(std::max(forwardDrive,0.f));
            gVehicleInputData.setAnalogBrake(0.0f);
            gVehicleInputData.setAnalogHandbrake(0.0f);
            gVehicleInputData.setAnalogSteer(horizontalDrive);
            if (gearedUp > 0.f && false)
            {
                gVehicleInputData.setGearUp(1.f);
                gearedUp--;
            }
            else
            {
                gVehicleInputData.setGearUp(0.f);
            }

            gVehicleInputData.setGearDown(0.0f);

            PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs
            (gPadSmoothingData, gSteerVsForwardSpeedTable, gVehicleInputData, 0.16f / SIM_STEPS, false, (PxVehicleDrive4W&)*mVehicles[0]);

            if (NULL == mSqWheelRaycastBatchQuery)
            {
                mSqWheelRaycastBatchQuery = mSqData->setUpBatchedSceneQuery(gScene_);
            }

            PxVehicleSuspensionRaycasts(mSqWheelRaycastBatchQuery, mNumVehicles, mVehicles, mSqData->getRaycastQueryResultBufferSize(), mSqData->getRaycastQueryResultBuffer());
                        


            PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];
            PxVehicleWheelQueryResult vehicleQueryResults[1] = { { wheelQueryResults, mVehicles[0]->mWheelsSimData.getNbWheels() } };
            physx::PxVehicleUpdates(0.16f / SIM_STEPS, GRAVITY, *mSurfaceTirePairs, 1, mVehicles, vehicleQueryResults);

        }


        gScene_->simulate(0.16f / SIM_STEPS);
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

    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        forwardDrive = (float)value / 32768;
        break; 

    case SDL_CONTROLLER_AXIS_LEFTX:
        horizontalDrive = (float)value / -32768;
        break; 

    default:
        break;
    }


}

















PxConvexMesh* createConvexMesh(const PxVec3* verts, const PxU32 numVerts, PxPhysics& physics, PxCooking& cooking)
{
    // Create descriptor for convex mesh
    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = numVerts;
    convexDesc.points.stride = sizeof(PxVec3);
    convexDesc.points.data = verts;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxConvexMesh* convexMesh = NULL;
    PxDefaultMemoryOutputStream buf;
    if (cooking.cookConvexMesh(convexDesc, buf))
    {
        PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
        convexMesh = physics.createConvexMesh(id);
    }

    return convexMesh;
}

PxConvexMesh* createCylinderConvexMesh(const PxF32 width, const PxF32 radius, const PxU32 numCirclePoints, PxPhysics& physics, PxCooking& cooking)
{
#define  MAX_NUM_VERTS_IN_CIRCLE 16
    PX_ASSERT(numCirclePoints<MAX_NUM_VERTS_IN_CIRCLE);
    PxVec3 verts[2 * MAX_NUM_VERTS_IN_CIRCLE];
    PxU32 numVerts = 2 * numCirclePoints;
    const PxF32 dtheta = 2 * PxPi / (1.0f*numCirclePoints);
    for (PxU32 i = 0; i<MAX_NUM_VERTS_IN_CIRCLE; i++)
    {
        const PxF32 theta = dtheta*i;
        const PxF32 cosTheta = radius*PxCos(theta);
        const PxF32 sinTheta = radius*PxSin(theta);
        verts[2 * i + 0] = PxVec3(-0.5f*width, cosTheta, sinTheta);
        verts[2 * i + 1] = PxVec3(+0.5f*width, cosTheta, sinTheta);
    }

    return createConvexMesh(verts, numVerts, physics, cooking);
}

PxConvexMesh* createWheelConvexMesh(const PxVec3* verts, const PxU32 numVerts, PxPhysics& physics, PxCooking& cooking)
{
    //Extract the wheel radius and width from the aabb of the wheel convex mesh.
    PxVec3 wheelMin(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
    PxVec3 wheelMax(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);
    for (PxU32 i = 0; i<numVerts; i++)
    {
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



void PhysicsSystem::handle_add_example_ship(const Event& e)
{

    if (mNumVehicles) return;

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
    auto& vehicle = dynamic_objects_.back();
    vehicle.set_mesh(gPhysics_, gCooking_, mesh);
    auto mat = vehicle.get_material();
    auto mesh_mesh = vehicle.get_mesh();


    float s = 0.2f;
    PxVec3 verts[8] = {
        {s,s,s}, {-s,s,s},{ s,s,-s },{ -s,s,-s },
       { s,-s,s },{ -s,-s,s },{ s,-s,-s },{ -s,-s,-s }
    };

    PxConvexMesh* wheel_mesh[4] = {
        createWheelConvexMesh(verts,8,*gPhysics_,*gCooking_),
        createWheelConvexMesh(verts,8,*gPhysics_,*gCooking_),
        createWheelConvexMesh(verts, 8, *gPhysics_, *gCooking_),
        createWheelConvexMesh(verts,8,*gPhysics_,*gCooking_) };

    //mesh_mesh = createWheelConvexMesh(verts, 8, *gPhysics_, *gCooking_);

    PxVec3 wheelCenterOffsets[4];
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT] = physx::PxVec3(-1.5f, 0.5f, 1.f);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] = physx::PxVec3(1.5f, 0.5f, 1.f);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT] = physx::PxVec3(-1.5f, 0.5f, -1.f);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT] = physx::PxVec3(1.5f, 0.5f, -1.f);

    create4WVehicle(*gScene_, *gPhysics_, *gCooking_, *mat, 1500.f, wheelCenterOffsets, mesh_mesh, wheel_mesh, transform, true);
    vehicle.set_actor(mVehicles[mNumVehicles - 1]->getRigidDynamicActor());
    vehicle.set_transform(transform);
}




















static PxF32 gTireFrictionMultipliers[MAX_NUM_SURFACE_TYPES][MAX_NUM_TIRE_TYPES] =
{
    //NORMAL,	WORN
    { 1.00f,		1.1f }//TARMAC
};


PxVehicleDrivableSurfaceToTireFrictionPairs* createFrictionPairs(const PxMaterial* defaultMaterial)
{
    PxVehicleDrivableSurfaceType surfaceTypes[1];
    surfaceTypes[0].mType = 0;

    const PxMaterial* surfaceMaterials[1];
    surfaceMaterials[0] = defaultMaterial;

    PxVehicleDrivableSurfaceToTireFrictionPairs* surfaceTirePairs =
        PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(1, 1);

    surfaceTirePairs->setup(1, 1, surfaceMaterials, surfaceTypes);

    for (PxU32 i = 0; i < 1; i++)
    {
        for (PxU32 j = 0; j < 1; j++)
        {
            surfaceTirePairs->setTypePairFriction(i, j, 1.f);
        }
    }
    return surfaceTirePairs;
}

void PhysicsSystem::handle_add_terrain(const Event& e)
{
    int object_id = e.get_value<int>("object_id", -1);
    assert(object_id != -1);

    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/Terrain.obj");

    static_objects_.emplace_back(object_id);
    static_objects_.back().set_mesh(gPhysics_, gCooking_, mesh);
   

    mSurfaceTirePairs = createFrictionPairs(static_objects_.back().get_material());

    gScene_->addActor(*static_objects_.back().get_actor());
}