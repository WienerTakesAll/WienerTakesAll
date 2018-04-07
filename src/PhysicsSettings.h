#pragma once

#include <string>

#include "PxPhysicsAPI.h"
#include "vehicle/PxVehicleTireFriction.h"
#include "vehicle/PxVehicleWheels.h"

class PhysicsSettings {
public:
    PhysicsSettings();
    physx::PxVec3 gravity;
    physx::PxVehiclePadSmoothingData g_pad_smoothing_data;
    physx::PxF32 g_steer_vs_forward_speed_data[2 * 8];
    physx::PxFixedSizeLookupTable<8> g_steer_vs_forward_speed_table;
    std::string vehicle_mesh_asset_path;
    float vehicle_mass;
    float wheel_mass;
    std::string arena_mesh;
    std::string basket_mesh;
    std::string charcoal_mesh;
    float arena_tire_friction;
    physx::PxVec3 wheel_center_offsets[4];
};