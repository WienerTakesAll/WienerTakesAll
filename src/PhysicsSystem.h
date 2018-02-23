#pragma once

#include <assert.h>

#include "PxPhysicsAPI.h"

#include "PhysicsComponent.h"
#include "EventSystem.h"
#include "FrictionPairService.h"
#include "VehicleControls.h"

class AssetManager;
class PhysicsSettings;
class VehicleWheelQueryResults;
class VehicleSceneQueryData;

#define MAX_NUM_4W_VEHICLES 10

class PhysicsSystem : public EventSystem<PhysicsSystem> {
public:
    PhysicsSystem(AssetManager&, PhysicsSettings&);
    ~PhysicsSystem();

    void update();

private:
    void handle_add_vehicle(const Event& e);
    void handle_add_arena(const Event& e);
    void handle_vehicle_control(const Event& e);

    void create_4w_vehicle(
        const physx::PxMaterial& material,
        const physx::PxF32 chassisMass,
        const physx::PxVec3* wheelCentreOffsets4,
        physx::PxConvexMesh* chassisConvexMesh,
        physx::PxConvexMesh** wheelConvexMeshes4,
        const physx::PxTransform& startTransform,
        const bool useAutoGearFlag
    );

    std::vector<PhysicsComponent<false>> dynamic_objects_;
    std::vector<PhysicsComponent<true>> static_objects_;

    physx::PxDefaultAllocator g_allocator_;
    physx::PxDefaultErrorCallback g_error_callback_;
    physx::PxFoundation* g_foundation_;
    physx::PxTolerancesScale g_scale_;
    physx::PxPvd* g_pvd_;
    physx::PxPhysics* g_physics_;
    physx::PxCooking* g_cooking_;
    physx::PxScene* g_scene_;

    // Data structures to keep track of vehicles
    physx::PxVehicleWheels* vehicles_[MAX_NUM_4W_VEHICLES];
    physx::PxVehicleWheelQueryResult vehicle_wheel_query_results_[MAX_NUM_4W_VEHICLES];
    physx::PxU32 num_vehicles_;
    std::vector<VehicleControls> vehicle_controls_;

    // Cached simulation data of focus vehicle in 4W mode.
    physx::PxVehicleWheelsSimData* wheels_sim_data_4w_;
    physx::PxVehicleDriveSimData4W drive_sim_data_4w_;

    VehicleWheelQueryResults* wheel_query_results;
    VehicleSceneQueryData* sq_data_;
    physx::PxBatchQuery* sq_wheel_raycast_batch_query_;

    AssetManager& asset_manager_;
    PhysicsSettings& settings_;

    physx::PxMaterial* arena_material_;

    FrictionPairService friction_pair_service_;
};
