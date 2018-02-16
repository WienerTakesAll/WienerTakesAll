#include "PhysicsComponent.h"
#include "VehicleSceneQueryData.h"

using namespace physx;

VehicleSceneQueryData* VehicleSceneQueryData::allocate(const PxU32 maxNumWheels) {
#define SIZEALIGN16(size) (((unsigned)(size)+15)&((unsigned)(~15)));

    const PxU32 size0 = SIZEALIGN16(sizeof(VehicleSceneQueryData));
    const PxU32 size1 = SIZEALIGN16(sizeof(PxRaycastQueryResult) * maxNumWheels);
    const PxU32 size2 = SIZEALIGN16(sizeof(PxRaycastHit) * maxNumWheels);
    const PxU32 size = size0 + size1 + size2;
    VehicleSceneQueryData* sqData = (VehicleSceneQueryData*)malloc(size);
    sqData->init();
    PxU8* ptr = (PxU8*)sqData;
    ptr += size0;
    sqData->sq_results_ = (PxRaycastQueryResult*)ptr;
    sqData->nb_sq_results_ = maxNumWheels;
    ptr += size1;
    sqData->sq_hit_buffer_ = (PxRaycastHit*)ptr;
    ptr += size2;
    sqData->num_queries_ = maxNumWheels;
    return sqData;

#undef SIZEALIGN16

}

PxBatchQuery* VehicleSceneQueryData::setup_batched_scene_query(PxScene* scene) {
    PxBatchQueryDesc sqDesc(nb_sq_results_, 0, 0);
    sqDesc.queryMemory.userRaycastResultBuffer = sq_results_;
    sqDesc.queryMemory.userRaycastTouchBuffer = sq_hit_buffer_;
    sqDesc.queryMemory.raycastTouchBufferSize = num_queries_;
    sqDesc.preFilterShader = pre_filter_shader_;
    return scene->createBatchQuery(sqDesc);
}

PxRaycastQueryResult* VehicleSceneQueryData::get_raycast_query_result_buffer() {
    return sq_results_;
}

PxU32 VehicleSceneQueryData::get_raycast_query_result_buffer_size() const {
    return num_queries_;
}

void VehicleSceneQueryData::set_pre_filter_shader(PxBatchQueryPreFilterShader preFilterShader) {
    pre_filter_shader_ = preFilterShader;
}

VehicleSceneQueryData::VehicleSceneQueryData() {
    init();
}

VehicleSceneQueryData::~VehicleSceneQueryData() {

}

void VehicleSceneQueryData::init() {
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
