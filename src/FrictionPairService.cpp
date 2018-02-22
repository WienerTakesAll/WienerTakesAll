#include <assert.h>

#include "FrictionPairService.h"

namespace {
    const float GARBAGE_VALUE = -1.f;
}

FrictionPairService::FrictionPairService(
    float standard_to_typical_friction,
    physx::PxMaterial* typical_material)
: friction_data_(SurfaceTypes::NUM_SURFACE_TYPES) {
    // initialize friction_data sub vectors
    for(auto& surface_pair : friction_data_) {
        surface_pair = std::vector<float>(TireTypes::NUM_TIRE_TYPES, GARBAGE_VALUE);
    }

    set_materials(typical_material);
    set_friction_data(standard_to_typical_friction);
}

FrictionPairService::~FrictionPairService() {

}

const physx::PxVehicleDrivableSurfaceToTireFrictionPairs& FrictionPairService::get_friction_pairs() const {
    return *friction_pairs_;
}

void FrictionPairService::set_materials(physx::PxMaterial* typical_material) {
    // index one
    surface_materials_.push_back(typical_material);
    initialize_friction_pairs();
}

void FrictionPairService::set_friction_data(float standard_to_typical_friction) {
    // set data
    friction_data_[SurfaceTypes::TYPICAL][TireTypes::STANDARD] = standard_to_typical_friction;
    initialize_friction_pairs();
}

void FrictionPairService::initialize_friction_pairs() {
    // check data size matches enums
    // must match, else if people use enum values not in array
    // will be bad times
    assert(friction_data_.size() == SurfaceTypes::NUM_SURFACE_TYPES);
    for(auto& surface_pair : friction_data_) {
        assert(surface_pair.size() == TireTypes::NUM_TIRE_TYPES);
    }

    assert(surface_materials_.size() == SurfaceTypes::NUM_SURFACE_TYPES);

    // set actual pairs
    physx::PxVehicleDrivableSurfaceType surface_types[SurfaceTypes::NUM_SURFACE_TYPES];
    for(int i = 0; i < SurfaceTypes::NUM_SURFACE_TYPES; i++) {
        surface_types[i].mType = i;
    }

    const physx::PxMaterial* surface_materials_array[SurfaceTypes::NUM_SURFACE_TYPES];
    for(int i = 0; i < surface_materials_.size(); i++) {
        surface_materials_array[i] = surface_materials_[i];
    }

    friction_pairs_ = physx::PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(
        SurfaceTypes::NUM_SURFACE_TYPES,
        TireTypes::NUM_TIRE_TYPES
    );

    friction_pairs_->setup(
        TireTypes::NUM_TIRE_TYPES,
        SurfaceTypes::NUM_SURFACE_TYPES,
        surface_materials_array,
        surface_types);

    for (int i = 0; i < friction_data_.size(); i++) {
        for (int j = 0; j < friction_data_[i].size(); j++) {
            friction_pairs_->setTypePairFriction(i, j, friction_data_[i][j]);
        }
    }
}
