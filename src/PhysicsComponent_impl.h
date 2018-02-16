#pragma once

#include <assert.h>

using namespace physx;

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
void PhysicsComponent<static_actor>::set_mesh(
    physx::PxPhysics* physics,
    physx::PxCooking* cooking,
    MeshAsset* mesh
) {
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

    assert(meshDesc.isValid());

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
void PhysicsComponent<static_actor>::setup_drive_sim(
    physx::PxVehicleDriveSimData4W& driveSimData,
    physx::PxVehicleWheelsSimData* wheelsSimData
) {
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
