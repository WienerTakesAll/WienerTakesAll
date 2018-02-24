#include "PhysicsSettings.h"

PhysicsSettings::PhysicsSettings()
    : gravity(physx::PxVec3 (0.0f, -0.1f, 0.0f))
    , g_pad_smoothing_data( {
    {
        6.0f,    //rise rate eANALOG_INPUT_ACCEL
        6.0f,    //rise rate eANALOG_INPUT_BRAKE
        12.0f,    //rise rate eANALOG_INPUT_HANDBRAKE
        2.5f,    //rise rate eANALOG_INPUT_STEER_LEFT
        2.5f,    //rise rate eANALOG_INPUT_STEER_RIGHT
    }, {
        10.0f,    //fall rate eANALOG_INPUT_ACCEL
        10.0f,    //fall rate eANALOG_INPUT_BRAKE
        12.0f,    //fall rate eANALOG_INPUT_HANDBRAKE
        5.0f,    //fall rate eANALOG_INPUT_STEER_LEFT
        5.0f    //fall rate eANALOG_INPUT_STEER_RIGHT
    }
})
, vehicle_mesh_asset_path("assets/models/carBoxModel.obj")
, vehicle_mass(1500.f)
, arena_mesh("assets/models/Terrain.obj")
, arena_tire_friction(1.f) {
    physx::PxF32 g_steer_vs_forward_speed_data[2 * 8] = {
        0.0f,        0.75f,
        5.0f,        0.75f,
        30.0f,        0.125f,
        120.0f,        0.1f,
        PX_MAX_F32, PX_MAX_F32,
        PX_MAX_F32, PX_MAX_F32,
        PX_MAX_F32, PX_MAX_F32,
        PX_MAX_F32, PX_MAX_F32
    };
    g_steer_vs_forward_speed_table = physx::PxFixedSizeLookupTable<8>(g_steer_vs_forward_speed_data, 4);
}
