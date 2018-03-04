#pragma once

#include <iostream>
#include <assert.h>

#include "PhysicsSystemUtils.h"
#include "CollisionFlags.h"

using namespace physx;

template <bool static_actor>
PhysicsComponent<static_actor>::PhysicsComponent(unsigned int id)
    : valid_(false)
    , id_(id)
    , g_actor_(nullptr)
	, id_ptr_(nullptr) {
}

template <bool static_actor>
PhysicsComponent<static_actor>::~PhysicsComponent() {
	if (id_ptr_ != nullptr)
	{
		// delete id_ptr_;
	}
}

template <bool static_actor>
bool PhysicsComponent<static_actor>::is_valid() const {
    return valid_;
}

template <bool static_actor>
unsigned int PhysicsComponent<static_actor>::get_id() const {
    return id_;
}

template <bool static_actor>
physx::PxMaterial* PhysicsComponent<static_actor>::get_material() const {
    return g_material_;
}

template <bool static_actor>
typename PhysicsComponent<static_actor>::PxMeshType* PhysicsComponent<static_actor>::get_mesh() const {
    return g_mesh_;
}

template <bool static_actor>
typename PhysicsComponent<static_actor>::PxMeshGeometryType* PhysicsComponent<static_actor>::get_mesh_geometry() const {
    return g_mesh_geometry_;
}

template <bool static_actor>
physx::PxShape* PhysicsComponent<static_actor>::get_mesh_shape() const {
    return g_mesh_shape_;
}

template <bool static_actor>
auto PhysicsComponent<static_actor>::get_actor() const {
    return g_actor_;
}

template <bool static_actor>
bool PhysicsComponent<static_actor>::is_vehicle() const {
    return is_vehicle_;
}

template <bool static_actor>
physx::PxVehicleDrive4W* PhysicsComponent<static_actor>::get_wheels() const {
    return g_drive_4w_;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_actor(physx::PxRigidDynamic* actor) {
    std::cout << "set_actor id: " << id_ << std::endl;
    g_actor_ = actor;

	if (id_ptr_ != nullptr) {
		delete id_ptr_;
	}
	id_ptr_ = new int(id_);
	g_actor_->userData = id_ptr_;

    std::cout << "set_actor g_actor_->userData: " << *(int*)g_actor_->userData << std::endl;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_mesh(
    physx::PxPhysics* physics,
    physx::PxCooking* cooking,
    MeshAsset* mesh
) {
    create_geometry(physics, cooking, mesh);

    physx::PxTransform phys_transform(0, 0, 0);

    // default material
    g_material_ = physics->createMaterial(0.5f, 0.5f, 0.5f);

    g_mesh_geometry_ = new PxMeshGeometryType(g_mesh_);

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
