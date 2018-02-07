#pragma once

template <bool static_actor>
PhysicsComponent<static_actor>::PhysicsComponent(unsigned int id)
    : valid_(false)
    , id_(id)
    , gActor_(nullptr) {
}

template <bool static_actor>
PhysicsComponent<static_actor>::~PhysicsComponent() {
}

template <bool static_actor>
bool PhysicsComponent<static_actor>::is_valid() {
    return valid_;
}

template <bool static_actor>
unsigned int PhysicsComponent<static_actor>::get_id() {
    return id_;
}

template <bool static_actor>
auto PhysicsComponent<static_actor>::get_actor() {
    return gActor_;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_mesh(physx::PxPhysics* physics, physx::PxCooking* cooking, MeshAsset* mesh) {

    physx::PxConvexMeshDesc meshDesc;


    std::vector<physx::PxVec3> physVerts;
    std::vector<physx::PxU32> physIndices;

    for (auto& vert : mesh->vertices_) {
        physx::PxVec3 point;

        point.x = vert.position_[0];
        point.y = vert.position_[1];
        point.z = vert.position_[2];

        physVerts.push_back(point);
    }

    for (auto& ind : mesh->indices_) {
        physx::PxU32 index;

        index = ind;

        physIndices.push_back(index);
    }

    meshDesc.points.count = physVerts.size();
    meshDesc.points.data = &physVerts.front();
    meshDesc.points.stride = sizeof(physx::PxVec3);

    meshDesc.flags.set(physx::PxConvexFlag::eCOMPUTE_CONVEX);

    /*
    meshDesc..count = physIndices.size() / 3;
    meshDesc.triangles.data = &physIndices.front();
    meshDesc.triangles.stride = 3 * sizeof(physx::PxU32);*/

    bool valid = meshDesc.isValid();

    physx::PxDefaultMemoryOutputStream writeBuffer;
    physx::PxTriangleMeshCookingResult::Enum result;

    physx::PxHullPolygon* meshPolygon = nullptr;

    physx::PxU32 nbVerts;
    physx::PxVec3* verts;
    physx::PxU32 nbIndices;
    physx::PxU32* indices;
    physx::PxU32 nbPolygons;

    bool status = cooking->cookConvexMesh(meshDesc, writeBuffer);
    if (!status)
    {
        return;
    }


    physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    gMesh_ = physics->createConvexMesh(readBuffer);

    physx::PxTransform physTransform(1, 0, 0);
    gMaterial_ = physics->createMaterial(0.1f, 0.1f, 0.1f);

    physx::PxConvexMeshGeometry meshGeometry(gMesh_);

    gMeshShape_ = physics->createShape(meshGeometry, *gMaterial_);


    createActor(physics, physTransform, gMeshShape_, 1.0f);

    valid_ = true;
}

template <bool static_actor>
void PhysicsComponent<static_actor>::set_transform(physx::PxTransform& transform) {
    gActor_->setGlobalPose(transform);
}