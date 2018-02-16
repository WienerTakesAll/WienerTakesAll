#pragma once

using namespace physx;

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
);

PxRigidDynamic* createVehicleActor4W(
    const PxVehicleChassisData& chassisData,
    PxConvexMesh** wheelConvexMeshes,
    PxConvexMesh* chassisConvexMesh,
    PxScene& scene,
    PxPhysics& physics,
    const PxMaterial& material
);

void computeWheelWidthsAndRadii(
    PxConvexMesh** wheelConvexMeshes,
    PxF32* wheelWidths,
    PxF32* wheelRadii
);

PxVec3 computeChassisAABBDimensions(const PxConvexMesh* chassisConvexMesh);

void createVehicle4WSimulationData(
    const PxF32 chassisMass, PxConvexMesh* chassisConvexMesh,
    const PxF32 wheelMass,
    PxConvexMesh** wheelConvexMeshes,
    const PxVec3* wheelCentreOffsets,
    PxVehicleWheelsSimData& wheelsData,
    PxVehicleDriveSimData4W& driveData,
    PxVehicleChassisData& chassisData
);

PxConvexMesh* createConvexMesh(
    const PxVec3* verts,
    const PxU32 numVerts,
    PxPhysics& physics,
    PxCooking& cooking
);

PxConvexMesh* createCylinderConvexMesh(
    const PxF32 width,
    const PxF32 radius,
    const PxU32 numCirclePoints,
    PxPhysics& physics,
    PxCooking& cooking
);

PxConvexMesh* createWheelConvexMesh(
    const PxVec3* verts,
    const PxU32 numVerts,
    PxPhysics& physics,
    PxCooking& cooking
);

PxVehicleDrivableSurfaceToTireFrictionPairs* createFrictionPairs(const PxMaterial* defaultMaterial);

void SampleVehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData);
