#include <iostream>

#include <glm/gtx/quaternion.hpp>
#include "PxPhysicsAPI.h"

#include "PhysicsSystemUtils.h"
#include "PhysicsComponent.h"


using namespace physx;

void setup_actor (
    PxRigidDynamic* vehicle_actor,
    const PxFilterData& veh_qry_filter_data,
    const PxGeometry** wheel_geometries,
    const PxTransform* wheel_local_poses,
    const PxU32 num_wheel_geometries,
    const PxMaterial* wheel_material,
    const PxFilterData& wheel_coll_filter_data,
    const PxGeometry** chassis_geometries,
    const PxTransform* chassis_local_poses,
    const PxU32 num_chassis_geometries,
    const PxMaterial* chassis_material,
    const PxFilterData& chassis_coll_filter_data,
    const PxVehicleChassisData& chassis_data,
    PxPhysics* physics
) {
    // Add all the wheel shapes to the actor.
    for (PxU32 i = 0; i < num_wheel_geometries; i++) {
        PxShape* wheel_shape = PxRigidActorExt::createExclusiveShape(*vehicle_actor, *wheel_geometries[i], *wheel_material);
        wheel_shape->setQueryFilterData(veh_qry_filter_data);
        wheel_shape->setSimulationFilterData(wheel_coll_filter_data);
        wheel_shape->setLocalPose(wheel_local_poses[i]);
    }

    // Add the chassis shapes to the actor.
    for (PxU32 i = 0; i < num_chassis_geometries; i++) {
        PxShape* chassis_shape = PxRigidActorExt::createExclusiveShape(*vehicle_actor, *chassis_geometries[i], *chassis_material);
        chassis_shape->setQueryFilterData(veh_qry_filter_data);
        chassis_shape->setSimulationFilterData(chassis_coll_filter_data);
        chassis_shape->setLocalPose(chassis_local_poses[i]);
    }

    vehicle_actor->setMass(chassis_data.mMass);
    vehicle_actor->setMassSpaceInertiaTensor(chassis_data.mMOI);
    vehicle_actor->setCMassLocalPose(PxTransform(chassis_data.mCMOffset, PxQuat(PxIdentity)));
}

PxRigidDynamic* create_4w_vehicle_actor(
    const PxVehicleChassisData& chassis_data,
    PxConvexMesh** wheel_convex_meshes,
    PxConvexMesh* chassis_convex_mesh,
    PxScene& scene,
    PxPhysics& physics,
    const PxMaterial& material
) {
    // We need a rigid body actor for the vehicle.
    // Don't forget to add the actor the scene after setting up the associated vehicle.
    PxRigidDynamic* vehicle_actor = physics.createRigidDynamic(PxTransform(PxIdentity));

    // We need to add wheel collision shapes, their local poses, a material for the wheels, and a simulation filter for the wheels.
    PxConvexMeshGeometry front_left_wheel_geom(wheel_convex_meshes[0]);
    PxConvexMeshGeometry front_right_wheel_geom(wheel_convex_meshes[1]);
    PxConvexMeshGeometry rear_left_wheel_geom(wheel_convex_meshes[2]);
    PxConvexMeshGeometry rear_right_wheel_geom(wheel_convex_meshes[3]);
    const PxGeometry* wheel_geometries[4] = { &front_left_wheel_geom, &front_right_wheel_geom, &rear_left_wheel_geom, &rear_right_wheel_geom };
    const PxTransform wheel_local_poses[4] = { PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity) };
    const PxMaterial& wheel_material = material;
    PxFilterData wheel_coll_filter_data;
    wheel_coll_filter_data.word0 = COLLISION_FLAG_WHEEL;
    wheel_coll_filter_data.word1 = COLLISION_FLAG_WHEEL_AGAINST;
    wheel_coll_filter_data.word3 = SAMPLEVEHICLE_UNDRIVABLE_SURFACE;

    // We need to add chassis collision shapes, their local poses, a material for the chassis, and a simulation filter for the chassis.
    PxConvexMeshGeometry chassis_convex_geom(chassis_convex_mesh);
    const PxGeometry* chassis_geoms[1] = { &chassis_convex_geom };
    const PxTransform chassis_local_poses[1] = { PxTransform(PxIdentity) };
    const PxMaterial& chassis_material = material;
    PxFilterData chassis_coll_filter_data;
    chassis_coll_filter_data.word0 = COLLISION_FLAG_CHASSIS;
    chassis_coll_filter_data.word1 = COLLISION_FLAG_CHASSIS_AGAINST;
    chassis_coll_filter_data.word3 = SAMPLEVEHICLE_UNDRIVABLE_SURFACE;


    // Create a query filter data for the car to ensure that cars
    // do not attempt to drive on themselves.
    PxFilterData vehicle_qry_filter_data;
    vehicle_setup_vehicle_shape_query_filter_data(&vehicle_qry_filter_data);

    // Set up the physx rigid body actor with shapes, local poses, and filters.
    setup_actor(
        vehicle_actor,
        vehicle_qry_filter_data,
        wheel_geometries,
        wheel_local_poses,
        4,
        &wheel_material,
        wheel_coll_filter_data,
        chassis_geoms,
        chassis_local_poses,
        1,
        &chassis_material,
        chassis_coll_filter_data,
        chassis_data,
        &physics);

    return vehicle_actor;
}

void compute_wheel_widths_and_radii(
    PxConvexMesh** wheel_convex_meshes,
    PxF32* wheel_widths,
    PxF32* wheel_radii
) {
    for (PxU32 i = 0; i < 4; i++) {
        const PxU32 num_wheel_verts = wheel_convex_meshes[i]->getNbVertices();
        const PxVec3* wheel_verts = wheel_convex_meshes[i]->getVertices();
        PxVec3 wheel_min(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
        PxVec3 wheel_max(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);

        for (PxU32 j = 0; j < num_wheel_verts; j++) {
            wheel_min.x = PxMin(wheel_min.x, wheel_verts[j].x);
            wheel_min.y = PxMin(wheel_min.y, wheel_verts[j].y);
            wheel_min.z = PxMin(wheel_min.z, wheel_verts[j].z);
            wheel_max.x = PxMax(wheel_max.x, wheel_verts[j].x);
            wheel_max.y = PxMax(wheel_max.y, wheel_verts[j].y);
            wheel_max.z = PxMax(wheel_max.z, wheel_verts[j].z);
        }

        wheel_widths[i] = wheel_max.x - wheel_min.x;
        wheel_radii[i] = PxMax(wheel_max.y, wheel_max.z) * 0.975f;
    }
}

PxVec3 compute_chassis_aabb_dimensions(const PxConvexMesh* chassis_convex_mesh) {
    const PxU32 num_chassis_verts = chassis_convex_mesh->getNbVertices();
    const PxVec3* chassis_verts = chassis_convex_mesh->getVertices();
    PxVec3 chassis_min(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
    PxVec3 chassis_max(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);

    for (PxU32 i = 0; i < num_chassis_verts; i++) {
        chassis_min.x = PxMin(chassis_min.x, chassis_verts[i].x);
        chassis_min.y = PxMin(chassis_min.y, chassis_verts[i].y);
        chassis_min.z = PxMin(chassis_min.z, chassis_verts[i].z);
        chassis_max.x = PxMax(chassis_max.x, chassis_verts[i].x);
        chassis_max.y = PxMax(chassis_max.y, chassis_verts[i].y);
        chassis_max.z = PxMax(chassis_max.z, chassis_verts[i].z);
    }

    const PxVec3 chassis_dims = chassis_max - chassis_min;
    return chassis_dims;
}



void create_4w_vehicle_simulation_data(
    const PxF32 chassis_mass,
    PxConvexMesh* chassis_convex_mesh,
    const PxF32 wheel_mass,
    PxConvexMesh** wheel_convex_meshes,
    const PxVec3* wheel_centre_offsets,
    PxVehicleWheelsSimData& wheels_data,
    PxVehicleDriveSimData4W& drive_data,
    PxVehicleChassisData& chassis_data
) {
    // Extract the chassis AABB dimensions from the chassis convex mesh.
    const PxVec3 chassis_dims = compute_chassis_aabb_dimensions(chassis_convex_mesh);

    // The origin is at the center of the chassis mesh.
    // Set the center of mass to be below this point and a little towards the front.
    const PxVec3 chassis_cm_offset = PxVec3(0.0f, -chassis_dims.y * 0.5f + .65f, 0.25f);

    // Now compute the chassis mass and moment of inertia.
    // Use the moment of inertia of a cuboid as an approximate value for the chassis moi.
    PxVec3 chassis_moi(
        (chassis_dims.y * chassis_dims.y + chassis_dims.z * chassis_dims.z)*chassis_mass / 12.0f,
        (chassis_dims.x * chassis_dims.x + chassis_dims.z * chassis_dims.z)*chassis_mass / 12.0f,
        (chassis_dims.x * chassis_dims.x + chassis_dims.y * chassis_dims.y)*chassis_mass / 12.0f
    );
    // A bit of tweaking here.  The car will have more responsive turning if we reduce the
    // y-component of the chassis moment of inertia.
    chassis_moi.y *= 0.8f;

    // Let's set up the chassis data structure now.
    chassis_data.mMass = chassis_mass;
    chassis_data.mMOI = chassis_moi;
    chassis_data.mCMOffset = chassis_cm_offset;

    // Compute the sprung masses of each suspension spring using a helper function.
    PxF32 suspension_sprung_masses[4];
    PxVehicleComputeSprungMasses(4, wheel_centre_offsets, chassis_cm_offset, chassis_mass, 1, suspension_sprung_masses);


    // Extract the wheel radius and width from the wheel convex meshes.
    PxF32 wheel_widths[4];
    PxF32 wheel_radii[4];
    compute_wheel_widths_and_radii(wheel_convex_meshes, wheel_widths, wheel_radii);

    // Now compute the wheel masses and inertias components around the axle's axis.
    // http://en.wikipedia.org/wiki/List_of_moments_of_inertia
    PxF32 wheel_mois[4];

    for (PxU32 i = 0; i < 4; i++) {
        wheel_mois[i] = 0.5f * wheel_mass * wheel_radii[i] * wheel_radii[i];
    }

    // Let's set up the wheel data structures now with radius, mass, and moi.
    PxVehicleWheelData wheels[4];

    for (PxU32 i = 0; i < 4; i++) {
        wheels[i].mRadius = wheel_radii[i] * 3.f;
        wheels[i].mMass = wheel_mass;
        wheels[i].mMOI = wheel_mois[i];
        wheels[i].mWidth = wheel_widths[i];
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

    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mSprungMass = suspension_sprung_masses[PxVehicleDrive4WWheelOrder::eFRONT_LEFT];
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mSprungMass = suspension_sprung_masses[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT];
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mSprungMass = suspension_sprung_masses[PxVehicleDrive4WWheelOrder::eREAR_LEFT];
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mSprungMass = suspension_sprung_masses[PxVehicleDrive4WWheelOrder::eREAR_RIGHT];

    // Set up the camber.
    // Remember that the left and right wheels need opposite camber so that the car preserves symmetry about the forward direction.
    // Set the camber to 0.0f when the spring is neither compressed or elongated.
    const PxF32 camber_angle_at_rest = 0.0;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtRest = camber_angle_at_rest;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtRest = -camber_angle_at_rest;
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtRest = camber_angle_at_rest;
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtRest = -camber_angle_at_rest;
    // Set the wheels to camber inwards at maximum droop (the left and right wheels almost form a V shape)
    const PxF32 camber_angle_at_max_droop = 0.001f;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxDroop = camber_angle_at_max_droop;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxDroop = -camber_angle_at_max_droop;
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxDroop = camber_angle_at_max_droop;
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxDroop = -camber_angle_at_max_droop;
    // Set the wheels to camber outwards at maximum compression (the left and right wheels almost form a A shape).
    const PxF32 camber_angle_at_max_compression = -0.001f;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxCompression = camber_angle_at_max_compression;
    susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxCompression = -camber_angle_at_max_compression;
    susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxCompression = camber_angle_at_max_compression;
    susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxCompression = -camber_angle_at_max_compression;

    // We need to set up geometry data for the suspension, wheels, and tires.
    // We already know the wheel centers described as offsets from the actor center and the center of mass offset from actor center.
    // From here we can approximate application points for the tire and suspension forces.
    // Lets assume that the suspension travel directions are absolutely vertical.
    // Also assume that we apply the tire and suspension forces 30cm below the center of mass.
    PxVec3 susp_travel_directions[4] = { PxVec3(0, -1, 0), PxVec3(0, -1, 0), PxVec3(0, -1, 0), PxVec3(0, -1, 0) };
    PxVec3 wheel_centre_cm_offsets[4];
    PxVec3 susp_force_app_cm_offsets[4];
    PxVec3 tire_force_app_cm_offsets[4];

    for (PxU32 i = 0; i < 4; i++) {
        wheel_centre_cm_offsets[i] = wheel_centre_offsets[i] - chassis_cm_offset;
        susp_force_app_cm_offsets[i] = PxVec3(wheel_centre_cm_offsets[i].x, 1.3f, wheel_centre_cm_offsets[i].z);
        tire_force_app_cm_offsets[i] = PxVec3(wheel_centre_cm_offsets[i].x, 1.3f, wheel_centre_cm_offsets[i].z);
    }

    // Now add the wheel, tire and suspension data.
    for (PxU32 i = 0; i < 4; i++) {
        wheels_data.setWheelData(i, wheels[i]);
        wheels_data.setTireData(i, tires[i]);
        wheels_data.setSuspensionData(i, susps[i]);
        wheels_data.setSuspTravelDirection(i, susp_travel_directions[i]);
        wheels_data.setWheelCentreOffset(i, wheel_centre_cm_offsets[i]);
        wheels_data.setSuspForceAppPointOffset(i, susp_force_app_cm_offsets[i]);
        wheels_data.setTireForceAppPointOffset(i, tire_force_app_cm_offsets[i]);
    }

    // Set the car to perform 3 sub-steps when it moves with a forwards speed of less than 5.0
    // and with a single step when it moves at speed greater than or equal to 5.0.
    wheels_data.setSubStepCount(5.0f, 3, 1);

    // Now set up the differential, engine, gears, clutch, and ackermann steering.

    // Diff
    PxVehicleDifferential4WData diff;
    diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
    drive_data.setDiffData(diff);

    // Engine
    PxVehicleEngineData engine;
    engine.mPeakTorque = 50.0f;
    engine.mMaxOmega = 60.0f; // approx 6000 rpm
    drive_data.setEngineData(engine);

    // Gears
    PxVehicleGearsData gears;
    gears.mSwitchTime = 0.5f;
    drive_data.setGearsData(gears);

    // Clutch
    PxVehicleClutchData clutch;
    clutch.mStrength = 10.0f;
    drive_data.setClutchData(clutch);

    // Ackermann steer accuracy
    PxVehicleAckermannGeometryData ackermann;
    ackermann.mAccuracy = 1.0f;
    ackermann.mAxleSeparation =
        wheel_centre_offsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].z
        - wheel_centre_offsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].z;
    ackermann.mFrontWidth =
        wheel_centre_offsets[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].x
        - wheel_centre_offsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].x;
    ackermann.mRearWidth =
        wheel_centre_offsets[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].x
        - wheel_centre_offsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].x;

    drive_data.setAckermannGeometryData(ackermann);
}

PxConvexMesh* create_convex_mesh(
    const PxVec3* verts,
    const PxU32 num_verts,
    PxPhysics& physics,
    PxCooking& cooking
) {

    // Create descriptor for convex mesh
    PxConvexMeshDesc convex_desc;
    convex_desc.points.count = num_verts;
    convex_desc.points.stride = sizeof(PxVec3);
    convex_desc.points.data = verts;
    convex_desc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxConvexMesh* convex_mesh = NULL;
    PxDefaultMemoryOutputStream buf;

    if (cooking.cookConvexMesh(convex_desc, buf)) {
        PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
        convex_mesh = physics.createConvexMesh(id);
    }

    return convex_mesh;
}

PxConvexMesh* create_cylinder_convex_mesh(
    const PxF32 width,
    const PxF32 radius,
    const PxU32 num_circle_points,
    PxPhysics& physics,
    PxCooking& cooking
) {
#define  MAX_NUM_VERTS_IN_CIRCLE 16
    PX_ASSERT(num_circle_points < MAX_NUM_VERTS_IN_CIRCLE);
    PxVec3 verts[2 * MAX_NUM_VERTS_IN_CIRCLE];
    PxU32 numVerts = 2 * num_circle_points;
    const PxF32 dtheta = 2 * PxPi / (1.0f * num_circle_points);

    for (PxU32 i = 0; i < MAX_NUM_VERTS_IN_CIRCLE; i++) {
        const PxF32 theta = dtheta * i;
        const PxF32 cosTheta = radius * PxCos(theta);
        const PxF32 sinTheta = radius * PxSin(theta);
        verts[2 * i + 0] = PxVec3(-0.5f * width, cosTheta, sinTheta);
        verts[2 * i + 1] = PxVec3(+0.5f * width, cosTheta, sinTheta);
    }

    return create_convex_mesh(verts, numVerts, physics, cooking);

#undef MAX_NUM_VERTS_IN_CIRCLE

}

PxConvexMesh* create_wheel_convex_mesh(
    const PxVec3* verts,
    const PxU32 num_verts,
    PxPhysics& physics,
    PxCooking& cooking
) {

    //Extract the wheel radius and width from the aabb of the wheel convex mesh.
    PxVec3 wheel_min(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
    PxVec3 wheel_max(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);

    for (PxU32 i = 0; i < num_verts; i++) {
        wheel_min.x = PxMin(wheel_min.x, verts[i].x);
        wheel_min.y = PxMin(wheel_min.y, verts[i].y);
        wheel_min.z = PxMin(wheel_min.z, verts[i].z);
        wheel_max.x = PxMax(wheel_max.x, verts[i].x);
        wheel_max.y = PxMax(wheel_max.y, verts[i].y);
        wheel_max.z = PxMax(wheel_max.z, verts[i].z);
    }

    const PxF32 wheel_width = wheel_max.x - wheel_min.x;
    const PxF32 wheel_radius = PxMax(wheel_max.y, wheel_max.z);

    return create_cylinder_convex_mesh(wheel_width, wheel_radius, 8, physics, cooking);
}

void vehicle_setup_vehicle_shape_query_filter_data(PxFilterData* qry_filter_data) {
    if (0 != qry_filter_data->word3) {
        std::cerr << "word3 is reserved for filter data for vehicle raycast queries" << std::endl;
        assert(0 == qry_filter_data->word3);
    }

    qry_filter_data->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}
