#pragma once

#include <assert.h>

#include "PxPhysicsAPI.h"

#include "PhysicsComponent.h"
#include "EventSystem.h"

class AssetManager;
class PhysicsSettings;

#define MAX_NUM_4W_VEHICLES 10

using namespace physx;
class PhysicsSystem : public EventSystem<PhysicsSystem> {
public:
    PhysicsSystem(AssetManager&, PhysicsSettings&);
    ~PhysicsSystem();

    void update();

private:
    void handle_add_example_ship(const Event& e);
    void handle_add_terrain(const Event& e);
    void handle_key_press(const Event& e);

    void create_4w_vehicle(
        PxScene& scene,
        PxPhysics& physics,
        PxCooking& cooking,
        const PxMaterial& material,
        const PxF32 chassisMass,
        const PxVec3* wheelCentreOffsets4,
        PxConvexMesh* chassisConvexMesh,
        PxConvexMesh** wheelConvexMeshes4,
        const PxTransform& startTransform,
        const bool useAutoGearFlag);

    physx::PxDefaultAllocator g_allocator_;
    physx::PxDefaultErrorCallback g_error_callback_;
    physx::PxTolerancesScale g_scale_;
    physx::PxFoundation* g_foundation_;
    physx::PxPvd* g_pvd_;
    physx::PxPhysics* g_physics_;
    physx::PxCooking* g_cooking_;
    physx::PxScene* g_scene_;

    AssetManager& asset_manager_;
    PhysicsSettings& settings_;

    std::vector<PhysicsComponent<false>> dynamic_objects_;
    std::vector<PhysicsComponent<true>> static_objects_;

    float forward_drive_, horizontal_drive_, backward_drive_;
    bool hand_break_;

    // Array of all cars and report data for each car.
    PxVehicleWheels* vehicles_[MAX_NUM_4W_VEHICLES];
    PxVehicleWheelQueryResult vehicle_wheel_query_results_[MAX_NUM_4W_VEHICLES];
    PxU32 num_vehicles_;

    // sdk raycasts (for the suspension lines).
    class SampleVehicleSceneQueryData {
    public:

        // Allocate scene query data for up to maxNumWheels suspension raycasts.
        static SampleVehicleSceneQueryData* allocate(const PxU32 maxNumWheels);

        // Create a PxBatchQuery instance that will be used as a single batched raycast of multiple suspension lines of multiple vehicles
        PxBatchQuery* setup_batched_scene_query(PxScene* scene);

        // Get the buffer of scene query results that will be used by PxVehicleNWSuspensionRaycasts
        PxRaycastQueryResult* get_raycast_query_result_buffer() {
            return sq_results_;
        }

        // Get the number of scene query results that have been allocated for use by PxVehicleNWSuspensionRaycasts
        PxU32 get_raycast_query_result_buffer_size() const {
            return num_queries_;
        }

        // Set the pre-filter shader
        void set_pre_filter_shader(PxBatchQueryPreFilterShader preFilterShader) {
            pre_filter_shader_ = preFilterShader;
        }

    private:

        // One result for each wheel.
        PxRaycastQueryResult* sq_results_;
        PxU32 nb_sq_results_;

        // One hit for each wheel.
        PxRaycastHit* sq_hit_buffer_;

        // Filter shader used to filter drivable and non-drivable surfaces
        PxBatchQueryPreFilterShader pre_filter_shader_;

        // Maximum number of suspension raycasts that can be supported by the allocated buffers
        // assuming a single query and hit per suspension line.
        PxU32 num_queries_;

        void init() {
            pre_filter_shader_ = [](
                PxFilterData filterData0,
                PxFilterData filterData1,
                const void* constantBlock, PxU32 constantBlockSize,
                PxHitFlags & queryFlags
            )->PxQueryHitType::Enum {
                // filterData0 is the vehicle suspension raycast.
                // filterData1 is the shape potentially hit by the raycast.
                PX_UNUSED(queryFlags);
                PX_UNUSED(constantBlockSize);
                PX_UNUSED(constantBlock);
                PX_UNUSED(filterData0);

                if ((0 == (filterData1.word3 & SAMPLEVEHICLE_DRIVABLE_SURFACE))) {
                    return PxQueryHitType::eNONE;
                } else {
                    return PxQueryHitType::eBLOCK;
                }
            };
        }

        SampleVehicleSceneQueryData() {
            init();
        }

        ~SampleVehicleSceneQueryData() {
        }
    };

    SampleVehicleSceneQueryData* sq_data_;

    PxBatchQuery* sq_wheel_raycast_batch_query_;

    // Reports for each wheel.
    class SampleVehicleWheelQueryResults {
    public:

        // Allocate a buffer of wheel query results for up to maxNumWheels.
        static SampleVehicleWheelQueryResults* allocate(const PxU32 maxNumWheels);

        PxWheelQueryResult* add_vehicle(const PxU32 numWheels);

    private:

        // One result for each wheel.
        PxWheelQueryResult* wheel_query_results_;

        // Maximum number of wheels.
        PxU32 max_num_wheels_;

        // Number of wheels
        PxU32 num_wheels_;


        SampleVehicleWheelQueryResults()
            : wheel_query_results_(NULL), max_num_wheels_(0), num_wheels_(0) {
            init();
        }

        ~SampleVehicleWheelQueryResults() {
        }

        void init() {
            wheel_query_results_ = NULL;
            max_num_wheels_ = 0;
            num_wheels_ = 0;
        }
    };


    SampleVehicleWheelQueryResults* wheel_query_results;

    // Cached simulation data of focus vehicle in 4W mode.
    PxVehicleWheelsSimData* wheels_sim_data_4w_;
    PxVehicleDriveSimData4W drive_sim_data_4w_;
    bool is_3w_mode_;

    // Friction from combinations of tire and surface types.
    PxVehicleDrivableSurfaceToTireFrictionPairs* surface_tire_pairs_;
};