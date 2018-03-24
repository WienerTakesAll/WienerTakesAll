#include "AssetManager.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/gtc/type_ptr.hpp"

#include <iostream>

namespace {
    const int SDL_FLAGS = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;

    const int SCREEN_WIDTH = 1024;
    const int SCREEN_HEIGHT = 720;
}

AssetManager::AssetManager() {

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window_ = SDL_CreateWindow("WienerTakesAll",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               SCREEN_WIDTH,
                               SCREEN_HEIGHT,
                               SDL_FLAGS);

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


    for (size_t i = 0; i < scene->mNumMeshes; i++) {
        mesh_data.meshes_.emplace_back();
        auto& meshRef = mesh_data.meshes_.back();
        auto& mesh = scene->mMeshes[i];

        for (unsigned int f_i = 0; f_i < mesh->mNumFaces; f_i++) {
            auto& faces = mesh->mFaces[f_i];

            for (unsigned int i = 0; i < faces.mNumIndices; i++) {
                meshRef.indices_.emplace_back(faces.mIndices[i]);
            }

        }

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            MeshAsset::MeshData::VertexData vertex;

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


            meshRef.vertices_.push_back(vertex);
        }

    }

    if (mesh_data.meshes_.size() > 0) {
        mesh_data.valid_ = true;
    }

    for (auto& mesh_asset_data : mesh_data.meshes_) {
        construct_shadow_volume(mesh_asset_data);
    }
}

void AssetManager::load_texture_from_file(const std::string& file_path) {
    auto texture_map = texture_assets_.emplace(file_path, TextureAsset());

    if (!texture_map.second) {
        std::cerr << "AssetManager emplacement failed!" << std::endl;
        return;
    }

    TextureAsset& texture_data = texture_map.first->second;
    texture_data.load(file_path);
}

void AssetManager::load_shader_from_file(const std::string& file_path) {
    auto shader_map = shader_assets_.emplace(file_path, ShaderAsset());

    if (!shader_map.second) {
        std::cerr << "AssetManager emplacement failed!" << std::endl;
        return;
    }

    ShaderAsset& shader_data = shader_map.first->second;


    shader_data.load(file_path + ".vert", file_path + ".frag");
}



void AssetManager::construct_shadow_volume(MeshAsset::MeshData& mesh) {

    struct Edge {
        GLuint v1, v2;

        bool operator==(const Edge& other) const {
            return (v1 == other.v1 && v2 == other.v2) || (v2 == other.v1 && v1 == other.v2);
        }
    };

    struct EdgeHash {
        std::size_t operator()(const Edge& k) const {
            using std::size_t;
            using std::hash;
            using std::string;

            return ((hash<int>()(k.v1) ^ (hash<int>()(k.v2) << 1)) >> 1);
        }
    };





    for (auto& vert : mesh.vertices_) {
        MeshAsset::MeshData::VolumeVertexData v = { glm::vec4(vert.position_, 0), vert.normal_ };
        mesh.shadow_volume_vertices_.emplace_back(v);
    }

    GLuint infVert = mesh.shadow_volume_vertices_.size();

    MeshAsset::MeshData::VolumeVertexData v = { glm::vec4(0, 0, 0, 1), glm::vec3(0, 0, 0) };
    mesh.shadow_volume_vertices_.emplace_back(v);



    std::unordered_map<Edge, int, EdgeHash> edgeSet;


    for (GLuint i = 0; i + 2 < mesh.indices_.size(); i += 3) {


        auto make_face = [&mesh, &edgeSet](GLuint v1, GLuint v2, GLuint v3) {

            if (edgeSet.count({ v1, v3 }) == 0) {
                Edge e = { v1, v3 };
                edgeSet.emplace(e, 0);

                mesh.shadow_volume_indices_.emplace_back(v1);
                mesh.shadow_volume_indices_.emplace_back(v2);
                mesh.shadow_volume_indices_.emplace_back(v3);
            }
        };

        make_face(mesh.indices_[i], infVert, mesh.indices_[i + 1]);
        make_face(mesh.indices_[i + 1], infVert, mesh.indices_[i + 2]);
        make_face(mesh.indices_[i + 2], infVert, mesh.indices_[i]);
    }

}