#pragma once

#include <vector>

#include "GL/glew.h"
#include "SDL_opengl.h"

struct MeshAsset {
    struct MeshData
    {
        struct VertexData {
            float position_[3];
            float normal_[3];
            float colors_[3];
            float uv_[2];
        };
        std::vector<VertexData> vertices_;
        std::vector<GLuint> indices_;
    };

    std::vector<MeshData> meshes_;
};
