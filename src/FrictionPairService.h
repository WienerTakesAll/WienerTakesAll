#pragma once

#include <vector>

#include "PxPhysicsAPI.h"

enum TireTypes {
	STANDARD = 0,
	NUM_TIRE_TYPES
};

enum SurfaceTypes {
	TYPICAL = 0,
	NUM_SURFACE_TYPES
};

// For use by PhysicsSystem
class FrictionPairService {
public:
	FrictionPairService(
		float standard_to_typical_friction,
		const physx::PxMaterial& typical_material
	);
	~FrictionPairService();
	const physx::PxVehicleDrivableSurfaceToTireFrictionPairs& get_friction_pairs() const;
	void set_materials(const physx::PxMaterial& typical_material);
	void set_friction_data(float standard_to_typical_friction);

private:
	void initialize_friction_pairs();
	std::vector<physx::PxMaterial> surface_materials_;
	std::vector<std::vector<float>> friction_data_;
	physx::PxVehicleDrivableSurfaceToTireFrictionPairs* friction_pairs_;
};