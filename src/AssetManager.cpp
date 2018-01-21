#include "AssetManager.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/gtc/type_ptr.hpp"

#include <iostream>

AssetManager::AssetManager() {
}


AssetManager::~AssetManager() {
}


MeshAsset* AssetManager::get_mesh_asset(const std::string& filepath) {
    auto asset = mesh_assets_.find(filepath);

    if (asset == mesh_assets_.end()) {
        load_mesh_from_file(filepath);
        asset = mesh_assets_.find(filepath);
    }

    return &asset->second;
}

void AssetManager::load_mesh_from_file(const std::string& file_path) {
    Assimp::Importer importer;

    auto mesh_map = mesh_assets_.emplace(file_path, MeshAsset());

    if (!mesh_map.second) {
        std::cout << "AssetManager emplacement failed!" << std::endl;
    }

    MeshAsset& mesh_data = mesh_map.first->second;

    const aiScene* scene = importer.ReadFile
                           ( file_path,
                             aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (!scene) {
        std::cout << importer.GetErrorString();
        return;
    }

    if (!scene->HasMeshes()) {
        std::cout << "No mesh found in model " << file_path << std::endl;
    }

    auto& mesh = scene->mMeshes[0];

    for (unsigned int f_i = 0; f_i < mesh->mNumFaces; f_i++) {
        auto& faces = mesh->mFaces[f_i];

        for (unsigned int i = 0; i < faces.mNumIndices; i++) {
            mesh_data.indices_.emplace_back(faces.mIndices[i]);
        }

    }

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        MeshAsset::VertexData vertex;

        //Load vertex positions
        vertex.position_[0] = mesh->mVertices[i].x;
        vertex.position_[1] = mesh->mVertices[i].y;
        vertex.position_[2] = mesh->mVertices[i].z;

        //Load normals
        if (mesh->HasNormals()) {
            vertex.normal_[0] = mesh->mNormals[i].x;
            vertex.normal_[1] = mesh->mNormals[i].y;
            vertex.normal_[2] = mesh->mNormals[i].z;
        }

        //Load colors
        if (mesh->GetNumColorChannels()) {
            vertex.colors_[0] = mesh->mColors[i][0].r;
            vertex.colors_[1] = mesh->mColors[i][0].g;
            vertex.colors_[2] = mesh->mColors[i][0].b;
        }

        //Load uvs
        if (mesh->HasTextureCoords(0)) {
            vertex.uv_[0] = mesh->mTextureCoords[i][0].x;
            vertex.uv_[1] = mesh->mTextureCoords[i][0].y;
        }

        mesh_data.vertices_.push_back(vertex);
    }
}
