#pragma once


template <bool static_actor>
PhysicsComponent<static_actor>::PhysicsComponent(unsigned int id)
    : valid_(false)
    , id_(id)
    , gActor_(nullptr) {
}

template <bool static_actor>
PhysicsComponent<static_actor>::~PhysicsComponent() {
}

template <bool static_actor>
bool PhysicsComponent<static_actor>::is_valid() {
    return valid_;
}

template <bool static_actor>
unsigned int PhysicsComponent<static_actor>::get_id() {
    return id_;
}

template <bool static_actor>
auto PhysicsComponent<static_actor>::get_actor() {
    return gActor_;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_actor(physx::PxRigidDynamic* actor) {
    gActor_ = actor;
}

template <bool static_actor>
physx::PxMaterial* PhysicsComponent<static_actor>::get_material() {
    return gMaterial_;
}

template <bool static_actor>
physx::PxConvexMesh* PhysicsComponent<static_actor>::get_mesh() {
    return gMesh_;
}

template <bool static_actor>
physx::PxVehicleDrive4W* PhysicsComponent<static_actor>::get_wheels() {
    return gDrive4W_;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_mesh(physx::PxPhysics* physics, physx::PxCooking* cooking, MeshAsset* mesh) {


    physx::PxConvexMeshDesc meshDesc;


    std::vector<physx::PxVec3> physVerts;
    std::vector<physx::PxU32> physIndices;

    for (auto& mesh_data : mesh->meshes_) {
        for (auto& vert : mesh_data.vertices_) {
            physx::PxVec3 point;

            point.x = vert.position_[0];
            point.y = vert.position_[1];
            point.z = vert.position_[2];

            physVerts.push_back(point);
        }
    }

    for (auto& mesh_data : mesh->meshes_) {
        for (auto& ind : mesh_data.indices_) {
            physx::PxU32 index;

            index = ind;

            physIndices.push_back(index);
        }
    }




    meshDesc.points.count = physVerts.size();
    meshDesc.points.data = &physVerts.front();
    meshDesc.points.stride = sizeof(physx::PxVec3);

    meshDesc.flags.set(physx::PxConvexFlag::eCOMPUTE_CONVEX);

    bool valid = meshDesc.isValid();

    physx::PxDefaultMemoryOutputStream writeBuffer;

    physx::PxHullPolygon* meshPolygon = nullptr;

    bool status = cooking->cookConvexMesh(meshDesc, writeBuffer);

    if (!status) {
        return;
    }


    physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    gMesh_ = physics->createConvexMesh(readBuffer);

    physx::PxTransform physTransform(0, 0, 0);
    gMaterial_ = physics->createMaterial(5.f, 5.f, 5.f);

    gMeshGeometry_ = new physx::PxConvexMeshGeometry(gMesh_);

    gMeshShape_ = physics->createShape(*gMeshGeometry_, *gMaterial_, true);
    createActor(physics, physTransform, gMeshShape_, 1.0f);

    if (static_actor) {
        physx::PxFilterData filterData;
        filterData.word3 = SAMPLEVEHICLE_DRIVABLE_SURFACE;
        gMeshShape_->setQueryFilterData(filterData);

        filterData.word0 = COLLISION_FLAG_GROUND;
        filterData.word1 = COLLISION_FLAG_GROUND_AGAINST;
        filterData.word3 = 0;

        gMeshShape_->setSimulationFilterData(filterData);
    } else {
        physx::PxFilterData filterData;
        filterData.word0 = COLLISION_FLAG_WHEEL;
        filterData.word1 = COLLISION_FLAG_WHEEL_AGAINST;
        filterData.word3 = SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
        gMeshShape_->setQueryFilterData(filterData);
        gMeshShape_->setSimulationFilterData(filterData);
    }


    valid_ = true;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_transform(physx::PxTransform& transform) {
    static_assert(!static_actor, "Cannot set transform of a static actor");
    gActor_->setGlobalPose(transform);
}


template <bool static_actor>
void PhysicsComponent<static_actor>::create_vehicle(physx::PxPhysics* physics, physx::PxCooking* cooking, MeshAsset* mesh) {
    static_assert(!static_actor, "Vehicles cannot be static actors");

    physx::PxVec3 chassisCMOffset(0.f, 0.2f, 0.f);

    set_mesh(physics, cooking, mesh);
    gActor_ = physics->createRigidDynamic(physx::PxTransform(physx::PxIdentity));

    physx::PxFilterData wheelQryFilterData;
    wheelQryFilterData.word0 = static_cast<physx::PxU32>(CollisionFlags::WHEELS);
    wheelQryFilterData.word1 = static_cast<physx::PxU32>(CollisionFlags::TERRAIN);
    wheelQryFilterData.word3 = static_cast<physx::PxU32>(CollisionFlags::UNDRIVABLE_SURFACE);
    physx::PxFilterData chassisQryFilterData;
    chassisQryFilterData.word0 = static_cast<physx::PxU32>(CollisionFlags::WHEELS);
    chassisQryFilterData.word1 = static_cast<physx::PxU32>(CollisionFlags::TERRAIN);
    chassisQryFilterData.word3 = static_cast<physx::PxU32>(CollisionFlags::UNDRIVABLE_SURFACE);

    physx::PxFilterData wheelSimFilterData;
    wheelSimFilterData.word0 = static_cast<physx::PxU32>(CollisionFlags::WHEELS);
    wheelSimFilterData.word1 = static_cast<physx::PxU32>(CollisionFlags::TERRAIN);
    wheelSimFilterData.word3 = static_cast<physx::PxU32>(CollisionFlags::UNDRIVABLE_SURFACE);

    physx::PxFilterData chassisSimFilterData;
    chassisSimFilterData.word0 = static_cast<physx::PxU32>(CollisionFlags::WHEELS);
    chassisSimFilterData.word1 = static_cast<physx::PxU32>(CollisionFlags::TERRAIN);
    chassisSimFilterData.word3 = static_cast<physx::PxU32>(CollisionFlags::UNDRIVABLE_SURFACE);

    physx::PxVehicleWheelsSimData* wheelsSimData = physx::PxVehicleWheelsSimData::allocate(4);
    setup_wheels(wheelsSimData);

    physx::PxVehicleDriveSimData4W driveSimData;
    setup_drive_sim(driveSimData, wheelsSimData);

    //physx::PxBoxGeometry wheelGeometry(0.4f, 0.4f, 0.4f);
    physx::PxSphereGeometry wheelGeometry(0.2f);
    physx::PxVec3 wheelCenterOffsets[4];

    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT] = physx::PxVec3(-0.5, 0.5, 0.5);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] = physx::PxVec3(0.5, 0.5, 0.5);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT] = physx::PxVec3(-0.5, 0.5, -0.5);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT] = physx::PxVec3(0.5, 0.5, -0.5);

    for (physx::PxU32 i = 0; i < 4; i++) {
        physx::PxShape* wheelShape = physx::PxRigidActorExt::createExclusiveShape(*gActor_, wheelGeometry, *gMaterial_);
        wheelShape->setQueryFilterData(wheelQryFilterData);
        wheelShape->setSimulationFilterData(wheelSimFilterData);
        wheelShape->setLocalPose(physx::PxTransform(wheelCenterOffsets[i]));
    }

    //Add the chassis shapes to the actor.

    for (physx::PxU32 i = 0; i < 1; i++) {
        physx::PxShape* chassisShape = physx::PxRigidActorExt::createExclusiveShape(*gActor_, *gMeshGeometry_, *gMaterial_);
        chassisShape->setQueryFilterData(chassisQryFilterData);
        chassisShape->setSimulationFilterData(chassisSimFilterData);
        chassisShape->setLocalPose(physx::PxTransform(chassisCMOffset));
    }

    gActor_->setMass(1500.0f);
    gActor_->setMassSpaceInertiaTensor(physx::PxVec3(0.5f, 0.5f, 0.5f));
    gActor_->setCMassLocalPose(physx::PxTransform(chassisCMOffset));

    gDrive4W_ = physx::PxVehicleDrive4W::allocate(4);
    gDrive4W_->setup(physics, gActor_, *wheelsSimData, driveSimData, 0);

    wheelsSimData->free();
}


template <bool static_actor>
void PhysicsComponent<static_actor>::setup_wheels(physx::PxVehicleWheelsSimData* wheelsSimData) {
    using namespace physx;

    physx::PxVec3 chassisCMOffset(0.f, 0.2f, 0.f);

    physx::PxVehicleWheelData wheels[4];

    {
        //Set up the wheel data structures with mass, moi, radius, width.
        for (physx::PxU32 i = 0; i < 4; i++) {
            wheels[i].mMass = 100.0f;
            wheels[i].mMOI = 0.1f;
            wheels[i].mRadius = .1f;
            wheels[i].mWidth = .2f;
        }

        //Enable the handbrake for the rear wheels only.
        wheels[physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 4000.0f;
        wheels[physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 4000.0f;
        //Enable steering for the front wheels only.
        wheels[physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = physx::PxPi * 0.3333f;
        wheels[physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = physx::PxPi * 0.3333f;
    }

    //Set up the tires.
    physx::PxVehicleTireData tires[PX_MAX_NB_WHEELS];
    {
        //Set up the tires.
        for (physx::PxU32 i = 0; i < 4; i++) {
            tires[i].mType = 0;
        }
    }

    physx::PxVec3 wheelCenterOffsets[4];

    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_LEFT] = physx::PxVec3(-0.5, 0.5, -0.5);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] = physx::PxVec3(0.5, 0.5, -0.5);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_LEFT] = physx::PxVec3(-0.5, 0.5, 0.5);
    wheelCenterOffsets[physx::PxVehicleDrive4WWheelOrder::eREAR_RIGHT] = physx::PxVec3(0.5, 0.5, 0.5);

    //Set up the suspensions
    PxVehicleSuspensionData suspensions[PX_MAX_NB_WHEELS];
    {
        //Compute the mass supported by each suspension spring.
        PxF32 suspSprungMasses[PX_MAX_NB_WHEELS];
        PxVehicleComputeSprungMasses
        (4, wheelCenterOffsets,
         chassisCMOffset, 1500.f, 1, suspSprungMasses);

        //Set the suspension data.
        for (PxU32 i = 0; i < 4; i++) {
            suspensions[i].mMaxCompression = 0.3f;
            suspensions[i].mMaxDroop = 0.1f;
            suspensions[i].mSpringStrength = 35000.f;
            suspensions[i].mSpringDamperRate = 4500.0f;
            suspensions[i].mSprungMass = suspSprungMasses[i];
        }

        //Set the camber angles.
        const PxF32 camberAngleAtRest = 0.0;
        const PxF32 camberAngleAtMaxDroop = 0.01f;
        const PxF32 camberAngleAtMaxCompression = -0.01f;

        for (PxU32 i = 0; i < 4; i += 2) {
            suspensions[i + 0].mCamberAtRest = camberAngleAtRest;
            suspensions[i + 1].mCamberAtRest = -camberAngleAtRest;
            suspensions[i + 0].mCamberAtMaxDroop = camberAngleAtMaxDroop;
            suspensions[i + 1].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
            suspensions[i + 0].mCamberAtMaxCompression = camberAngleAtMaxCompression;
            suspensions[i + 1].mCamberAtMaxCompression = -camberAngleAtMaxCompression;
        }
    }

    //Set up the wheel geometry.
    PxVec3 suspTravelDirections[PX_MAX_NB_WHEELS];
    PxVec3 wheelCentreCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 suspForceAppCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 tireForceAppCMOffsets[PX_MAX_NB_WHEELS];
    {
        //Set the geometry data.
        for (PxU32 i = 0; i < 4; i++) {
            //Vertical suspension travel.
            suspTravelDirections[i] = PxVec3(0, -1, 0);

            //Wheel center offset is offset from rigid body center of mass.
            wheelCentreCMOffsets[i] =
                wheelCenterOffsets[i] - chassisCMOffset;

            //Suspension force application point 0.3 metres below
            //rigid body center of mass.
            suspForceAppCMOffsets[i] =
                PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);

            //Tire force application point 0.3 metres below
            //rigid body center of mass.
            tireForceAppCMOffsets[i] =
                PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);
        }
    }

    //Set up the filter data of the raycast that will be issued by each suspension.
    PxFilterData qryFilterData;
    qryFilterData.word0 = static_cast<physx::PxU32>(CollisionFlags::WHEELS);
    qryFilterData.word1 = static_cast<physx::PxU32>(CollisionFlags::TERRAIN);
    qryFilterData.word3 = static_cast<physx::PxU32>(CollisionFlags::DRIVABLE_SURFACE);

    //Set the wheel, tire and suspension data.
    //Set the geometry data.
    //Set the query filter data
    for (PxU32 i = 0; i < 4; i++) {
        wheelsSimData->setWheelData(i, wheels[i]);
        wheelsSimData->setTireData(i, tires[i]);
        wheelsSimData->setSuspensionData(i, suspensions[i]);
        wheelsSimData->setSuspTravelDirection(i, suspTravelDirections[i]);
        wheelsSimData->setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
        wheelsSimData->setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
        wheelsSimData->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
        wheelsSimData->setSceneQueryFilterData(i, qryFilterData);
        wheelsSimData->setWheelShapeMapping(i, i);
    }




}

template <bool static_actor>
void PhysicsComponent<static_actor>::setup_drive_sim(physx::PxVehicleDriveSimData4W& driveSimData, physx::PxVehicleWheelsSimData* wheelsSimData) {
    using namespace physx;
    //Diff
    PxVehicleDifferential4WData diff;
    diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
    driveSimData.setDiffData(diff);

    //Engine
    PxVehicleEngineData engine;
    engine.mPeakTorque = 500.0f;
    engine.mMaxOmega = 600.0f;//approx 6000 rpm
    driveSimData.setEngineData(engine);

    //Gears
    PxVehicleGearsData gears;
    gears.mSwitchTime = 0.5f;
    driveSimData.setGearsData(gears);

    //Clutch
    PxVehicleClutchData clutch;
    clutch.mStrength = 10.0f;
    driveSimData.setClutchData(clutch);

    //Ackermann steer accuracy
    PxVehicleAckermannGeometryData ackermann;
    ackermann.mAccuracy = 1.0f;
    ackermann.mAxleSeparation =
        wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).z -
        wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).z;
    ackermann.mFrontWidth =
        wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).x -
        wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).x;
    ackermann.mRearWidth =
        wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).x -
        wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).x;
    driveSimData.setAckermannGeometryData(ackermann);
}