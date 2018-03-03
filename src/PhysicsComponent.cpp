#include "PhysicsComponent.h"

template <>
void PhysicsComponent<true>::create_actor(
    physx::PxPhysics* physics,
    physx::PxTransform& transform,
    physx::PxShape* shape, physx::PxReal density
) {
    g_actor_ = physx::PxCreateStatic(*physics, transform, *shape);
    g_actor_->userData = &id_;
}

template <>
void PhysicsComponent<false>::create_actor(
    physx::PxPhysics* physics,
    physx::PxTransform& transform,
    physx::PxShape* shape,
    physx::PxReal density
) {
    g_actor_ = physics->createRigidDynamic(transform);
    g_actor_->userData = &id_;
}

template<>
void PhysicsComponent<true>::create_geometry(    physx::PxPhysics* physics,
        physx::PxCooking* cooking,
        MeshAsset* mesh) {


    physx::PxTriangleMeshDesc mesh_desc;

    std::vector<physx::PxVec3> phys_verts;
    std::vector<physx::PxU32> phys_indices;

    for (auto& mesh_data : mesh->meshes_) {
        int index_offset = phys_verts.size();

        for (auto& vert : mesh_data.vertices_) {
            physx::PxVec3 point;

            point.x = vert.position_[0];
            point.y = vert.position_[1];
            point.z = vert.position_[2];

            phys_verts.push_back(point);
        }

        for (auto& ind : mesh_data.indices_) {
            physx::PxU32 index;

            index = ind;

            phys_indices.push_back(index + index_offset);
        }
    }

    mesh_desc.points.count = phys_verts.size();
    mesh_desc.points.data = &phys_verts.front();
    mesh_desc.points.stride = sizeof(physx::PxVec3);

    mesh_desc.triangles.count = phys_indices.size();
    mesh_desc.triangles.data = &phys_indices.front();
    mesh_desc.triangles.stride = 3 * sizeof(physx::PxU32);



    assert(mesh_desc.isValid());

    physx::PxDefaultMemoryOutputStream write_buffer;

    bool status = cooking->cookTriangleMesh(mesh_desc, write_buffer);

    if (!status) {
        return;
    }

    physx::PxDefaultMemoryInputData read_buffer(write_buffer.getData(), write_buffer.getSize());
    g_mesh_ = physics->createTriangleMesh(read_buffer);

    // default material
    g_material_ = physics->createMaterial(5.f, 5.f, 5.f);

    g_mesh_geometry_ = new physx::PxTriangleMeshGeometry(g_mesh_);

}

template<>
void PhysicsComponent<false>::create_geometry(    physx::PxPhysics* physics,
        physx::PxCooking* cooking,
        MeshAsset* mesh) {

    physx::PxConvexMeshDesc mesh_desc;

    std::vector<physx::PxVec3> phys_verts;
    std::vector<physx::PxU32> phys_indices;

    for (auto& mesh_data : mesh->meshes_) {
        for (auto& vert : mesh_data.vertices_) {
            physx::PxVec3 point;

            point.x = vert.position_[0];
            point.y = vert.position_[1];
            point.z = vert.position_[2];

            phys_verts.push_back(point);
        }
    }

    for (auto& mesh_data : mesh->meshes_) {
        for (auto& ind : mesh_data.indices_) {
            physx::PxU32 index;

            index = ind;

            phys_indices.push_back(index);
        }
    }

    mesh_desc.points.count = phys_verts.size();
    mesh_desc.points.data = &phys_verts.front();
    mesh_desc.points.stride = sizeof(physx::PxVec3);

    mesh_desc.flags.set(physx::PxConvexFlag::eCOMPUTE_CONVEX);

    assert(mesh_desc.isValid());

    physx::PxDefaultMemoryOutputStream write_buffer;

    bool status = cooking->cookConvexMesh(mesh_desc, write_buffer);

    if (!status) {
        return;
    }

    physx::PxDefaultMemoryInputData read_buffer(write_buffer.getData(), write_buffer.getSize());
    g_mesh_ = physics->createConvexMesh(read_buffer);

    physx::PxTransform phys_transform(0, 0, 0);

    // default material
    g_material_ = physics->createMaterial(5.f, 5.f, 5.f);

    g_mesh_geometry_ = new physx::PxConvexMeshGeometry(g_mesh_);

}
