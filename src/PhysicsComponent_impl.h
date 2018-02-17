#pragma once

#include <assert.h>

#include "PhysicsSystemUtils.h"

using namespace physx;

template <bool static_actor>
PhysicsComponent<static_actor>::PhysicsComponent(unsigned int id)
    : valid_(false)
    , id_(id)
    , g_actor_(nullptr) {
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
    return g_actor_;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_actor(physx::PxRigidDynamic* actor) {
    g_actor_ = actor;
}

template <bool static_actor>
physx::PxMaterial* PhysicsComponent<static_actor>::get_material() {
    return g_material_;
}

template <bool static_actor>
physx::PxConvexMesh* PhysicsComponent<static_actor>::get_mesh() {
    return g_mesh_;
}

template <bool static_actor>
physx::PxVehicleDrive4W* PhysicsComponent<static_actor>::get_wheels() {
    return g_drive_4w_;
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

    bool status = cooking->cookConvexMesh(meshDesc, writeBuffer);

    if (!status) {
        return;
    }


    physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    g_mesh_ = physics->create_convex_mesh(readBuffer);

    physx::PxTransform physTransform(0, 0, 0);
    g_material_ = physics->createMaterial(5.f, 5.f, 5.f);

    g_mesh_geometry_ = new physx::PxConvexMeshGeometry(g_mesh_);

    g_mesh_shape_ = physics->createShape(*g_mesh_geometry_, *g_material_, true);
    createActor(physics, physTransform, g_mesh_shape_, 1.0f);

    if (static_actor) {
        physx::PxFilterData filterData;
        filterData.word3 = SAMPLEVEHICLE_DRIVABLE_SURFACE;
        g_mesh_shape_->setQueryFilterData(filterData);

        filterData.word0 = COLLISION_FLAG_GROUND;
        filterData.word1 = COLLISION_FLAG_GROUND_AGAINST;
        filterData.word3 = 0;

        g_mesh_shape_->setSimulationFilterData(filterData);
    } else {
        physx::PxFilterData filterData;
        filterData.word0 = COLLISION_FLAG_WHEEL;
        filterData.word1 = COLLISION_FLAG_WHEEL_AGAINST;
        filterData.word3 = SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
        g_mesh_shape_->setQueryFilterData(filterData);
        g_mesh_shape_->setSimulationFilterData(filterData);
    }


    valid_ = true;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_transform(physx::PxTransform& transform) {
    static_assert(!static_actor, "Cannot set transform of a static actor");
    g_actor_->setGlobalPose(transform);
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
