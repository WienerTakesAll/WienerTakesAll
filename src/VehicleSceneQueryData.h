#pragma once

#include "PxPhysicsAPI.h"

using namespace physx;

// sdk raycasts (for the suspension lines).
class VehicleSceneQueryData {
public:
    // Allocate scene query data for up to maxNumWheels suspension raycasts.
    static VehicleSceneQueryData* allocate(const PxU32 maxNumWheels);

    // Create a PxBatchQuery instance that will be used as a single batched raycast of multiple suspension lines of multiple vehicles
    PxBatchQuery* setup_batched_scene_query(PxScene* scene);

    // Get the buffer of scene query results that will be used by PxVehicleNWSuspensionRaycasts
    PxRaycastQueryResult* get_raycast_query_result_buffer();

    // Get the number of scene query results that have been allocated for use by PxVehicleNWSuspensionRaycasts
    PxU32 get_raycast_query_result_buffer_size() const;

    // Set the pre-filter shader
    void set_pre_filter_shader(PxBatchQueryPreFilterShader preFilterShader);

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

    VehicleSceneQueryData();
    ~VehicleSceneQueryData();
    void init();
};