#include "AssetManager.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/gtc/type_ptr.hpp"

#include <iostream>

AssetManager::AssetManager() {
    const int sdl_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    const int screen_width = 640;
    const int screen_height = 480;

    window_ = SDL_CreateWindow("WienerTakesAll",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               screen_width,
                               screen_height,
                               sdl_flags);

    if (window_ == NULL) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }

}


AssetManager::~AssetManager() {
}


SDL_Window* AssetManager::get_window() const {
    return window_;
}

MeshAsset* AssetManager::get_mesh_asset(const std::string& filepath) {
    auto asset = mesh_assets_.find(filepath);

    if (asset == mesh_assets_.end()) {
        load_mesh_from_file(filepath);
        asset = mesh_assets_.find(filepath);
    }

    return &asset->second;
}

TextureAsset* AssetManager::get_texture_asset(const std::string& file_path) {
    auto asset = texture_assets_.find(file_path);

    if (asset == texture_assets_.end()) {
        load_texture_from_file(file_path);
        asset = texture_assets_.find(file_path);
    }

    return &asset->second;
}

ShaderAsset* AssetManager::get_shader_asset(const std::string& file_path) {
    auto asset = shader_assets_.find(file_path);

    if (asset == shader_assets_.end()) {
        load_shader_from_file(file_path);
        asset = shader_assets_.find(file_path);
    }

    return &asset->second;
}

void AssetManager::load_mesh_from_file(const std::string& file_path) {
    Assimp::Importer importer;

    auto mesh_map = mesh_assets_.emplace(file_path, MeshAsset());

    if (!mesh_map.second) {
        std::cerr << "AssetManager emplacement failed!" << std::endl;
        return;
    }

    MeshAsset& mesh_data = mesh_map.first->second;
    mesh_data.valid_ = false;

    const aiScene* scene = importer.ReadFile
                           ( file_path,
                             aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (!scene) {
        std::cerr << importer.GetErrorString();
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
            vertex.uv_[0] = mesh->mTextureCoords[0][i].x;
            vertex.uv_[1] = mesh->mTextureCoords[0][i].y;
        }

        mesh_data.vertices_.push_back(vertex);
    }

    if (mesh_data.vertices_.size() > 0 && mesh_data.indices_.size() > 0) {
        mesh_data.valid_ = true;
    }
}

void AssetManager::load_texture_from_file(const std::string& file_path) {
    auto texture_map = texture_assets_.emplace(file_path, TextureAsset());

    if (!texture_map.second) {
        std::cerr << "AssetManager emplacement failed!" << std::endl;
        return;
    }

    TextureAsset& texture_data = texture_map.first->second;
    texture_data.valid_ = texture_data.load_texture(file_path);

    if (!texture_data.valid_) {
        std::cout << "Could not load texture " << file_path << std::endl;
    }
}

void AssetManager::load_shader_from_file(const std::string& file_path) {
    auto shader_map = shader_assets_.emplace(file_path, ShaderAsset());

    if (!shader_map.second) {
        std::cerr << "AssetManager emplacement failed!" << std::endl;
        return;
    }

    ShaderAsset& shader_data = shader_map.first->second;
    shader_data.valid_ = shader_data.load_shader(file_path + ".vert", file_path + ".frag");

    if (!shader_data.valid_) {
        std::cout << "Could not load shader " << file_path << std::endl;
    }
}