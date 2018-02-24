#pragma once

#include <assert.h>

#include "PhysicsSystemUtils.h"
#include "CollisionFlags.h"

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
    physx::PxConvexMeshDesc mesh_desc;

    std::vector<physx::PxVec3> phys_verts;
    std::vector<physx::PxU32> phys_indices;

    for (auto& mesh_data : mesh->meshes_) {
        for (auto& vert : mesh_data.vertices_) {
            physx::PxVec3 point;

            point.x = vert.position_[0];
            point.y = vert.position_[1];
            point.z = vert.position_[2];

            phys_verts.push_back(point);
        }
    }

    for (auto& mesh_data : mesh->meshes_) {
        for (auto& ind : mesh_data.indices_) {
            physx::PxU32 index;

            index = ind;

            phys_indices.push_back(index);
        }
    }

    mesh_desc.points.count = phys_verts.size();
    mesh_desc.points.data = &phys_verts.front();
    mesh_desc.points.stride = sizeof(physx::PxVec3);

    mesh_desc.flags.set(physx::PxConvexFlag::eCOMPUTE_CONVEX);

    assert(mesh_desc.isValid());

    physx::PxDefaultMemoryOutputStream write_buffer;

    bool status = cooking->cookConvexMesh(mesh_desc, write_buffer);

    if (!status) {
        return;
    }

    physx::PxDefaultMemoryInputData read_buffer(write_buffer.getData(), write_buffer.getSize());
    g_mesh_ = physics->createConvexMesh(read_buffer);

    physx::PxTransform phys_transform(0, 0, 0);

    // default material
    g_material_ = physics->createMaterial(5.f, 5.f, 5.f);

    g_mesh_geometry_ = new physx::PxConvexMeshGeometry(g_mesh_);

    g_mesh_shape_ = physics->createShape(*g_mesh_geometry_, *g_material_, true);
    create_actor(physics, phys_transform, g_mesh_shape_, 1.0f);

    if (static_actor) {
        physx::PxFilterData filter_data;
        filter_data.word3 = CollisionFlags::DRIVABLE_SURFACE;
        g_mesh_shape_->setQueryFilterData(filter_data);

        filter_data.word0 = CollisionFlags::GROUND;
        filter_data.word1 = CollisionFlags::GROUND_AGAINST;
        filter_data.word3 = 0;

        g_mesh_shape_->setSimulationFilterData(filter_data);
    } else {
        physx::PxFilterData filter_data;
        filter_data.word0 = CollisionFlags::WHEEL;
        filter_data.word1 = CollisionFlags::WHEEL_AGAINST;
        filter_data.word3 = CollisionFlags::UNDRIVABLE_SURFACE;
        g_mesh_shape_->setQueryFilterData(filter_data);
        g_mesh_shape_->setSimulationFilterData(filter_data);
    }


    valid_ = true;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_transform(physx::PxTransform& transform) {
    static_assert(!static_actor, "Cannot set transform of a static actor");
    g_actor_->setGlobalPose(transform);
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_material(physx::PxMaterial* material) {
    g_material_ = material;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::setup_drive_sim(
    physx::PxVehicleDriveSimData4W& drive_sim_data,
    physx::PxVehicleWheelsSimData* wheels_sim_data
) {
    //Diff
    PxVehicleDifferential4WData diff;
    diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
    drive_sim_data.setDiffData(diff);

    //Engine
    PxVehicleEngineData engine;
    engine.mPeakTorque = 500.0f;
    engine.mMaxOmega = 600.0f;//approx 6000 rpm
    drive_sim_data.setEngineData(engine);

    //Gears
    PxVehicleGearsData gears;
    gears.mSwitchTime = 0.5f;
    drive_sim_data.setGearsData(gears);

    //Clutch
    PxVehicleClutchData clutch;
    clutch.mStrength = 10.0f;
    drive_sim_data.setClutchData(clutch);

    //Ackermann steer accuracy
    PxVehicleAckermannGeometryData ackermann;
    ackermann.mAccuracy = 1.0f;
    ackermann.mAxleSeparation =
        wheels_sim_data->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).z -
        wheels_sim_data->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).z;
    ackermann.mFrontWidth =
        wheels_sim_data->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).x -
        wheels_sim_data->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).x;
    ackermann.mRearWidth =
        wheels_sim_data->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).x -
        wheels_sim_data->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).x;
    drive_sim_data.setAckermannGeometryData(ackermann);
}
