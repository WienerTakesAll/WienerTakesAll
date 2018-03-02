#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "GL/glew.h"
#include "SDL_opengl.h"
#include "SDL.h"

#include "MeshAsset.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"

class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    MeshAsset* get_mesh_asset(const std::string& file_path);
    TextureAsset* get_texture_asset(const std::string& file_path);
    ShaderAsset* get_shader_asset(const std::string& file_path);

    SDL_Window* get_window() const;

private:
    void load_mesh_from_file(const std::string& file_path);
    void load_texture_from_file(const std::string& file_path);
    void load_shader_from_file(const std::string& file_path);
    void construct_shadow_volume(MeshAsset::MeshData& mesh);
    std::unordered_map<std::string, MeshAsset> mesh_assets_;
    std::unordered_map<std::string, TextureAsset> texture_assets_;
    std::unordered_map<std::string, ShaderAsset> shader_assets_;

    SDL_Window* window_;
};

