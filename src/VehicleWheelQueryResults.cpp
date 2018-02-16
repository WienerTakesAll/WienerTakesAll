#include <assert.h>
#include <stdio.h>

#include "vehicle/PxVehicleTireFriction.h"
#include "vehicle/PxVehicleWheels.h"

#include "PhysicsComponent.h" // I have no idea why this is necessary, but it is
#include "VehicleWheelQueryResults.h"

using namespace physx;

VehicleWheelQueryResults* VehicleWheelQueryResults::allocate(const PxU32 maxNumWheels) {
    const PxU32 size = sizeof(VehicleWheelQueryResults) + sizeof(PxWheelQueryResult) * maxNumWheels;
    VehicleWheelQueryResults* resData = (VehicleWheelQueryResults*)malloc(size);
    resData->init();
    PxU8* ptr = (PxU8*)resData;
    ptr += sizeof(VehicleWheelQueryResults);
    resData->wheel_query_results_ = (PxWheelQueryResult*)ptr;
    ptr += sizeof(PxWheelQueryResult) * maxNumWheels;
    resData->max_num_wheels_ = maxNumWheels;

    for (PxU32 i = 0; i < maxNumWheels; i++) {
        new (&resData->wheel_query_results_[i]) PxWheelQueryResult();
    }

    return resData;
}

PxWheelQueryResult* VehicleWheelQueryResults::add_vehicle(const PxU32 numWheels) {
    PX_ASSERT((num_wheels_ + numWheels) <= max_num_wheels_);
    PxWheelQueryResult* r = &wheel_query_results_[num_wheels_];
    num_wheels_ += numWheels;
    return r;
}

VehicleWheelQueryResults::VehicleWheelQueryResults()
    : wheel_query_results_(NULL), max_num_wheels_(0), num_wheels_(0) {
    init();
}

VehicleWheelQueryResults::~VehicleWheelQueryResults() {
}

void VehicleWheelQueryResults::init() {
    wheel_query_results_ = NULL;
    max_num_wheels_ = 0;
    num_wheels_ = 0;
}
