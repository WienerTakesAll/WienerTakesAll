#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "GL/glew.h"
#include "SDL_opengl.h"

#include "MeshAsset.h"

class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    MeshAsset* get_mesh_asset(const std::string& filepath);
private:
    void load_mesh_from_file(const std::string& filepath);

    std::unordered_map<std::string, MeshAsset> mesh_assets;
};

