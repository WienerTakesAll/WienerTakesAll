#pragma once

#include "PxPhysicsAPI.h"

using namespace physx;

// results for each wheel
class VehicleWheelQueryResults {
public:
    // Allocate a buffer of wheel query results for up to maxNumWheels.
    static VehicleWheelQueryResults* allocate(const PxU32 maxNumWheels);

    PxWheelQueryResult* add_vehicle(const PxU32 numWheels);

private:
    // One result for each wheel.
    PxWheelQueryResult* wheel_query_results_;

    // Maximum number of wheels.
    PxU32 max_num_wheels_;

    // Number of wheels
    PxU32 num_wheels_;

    VehicleWheelQueryResults();
    ~VehicleWheelQueryResults();
    void init();
};