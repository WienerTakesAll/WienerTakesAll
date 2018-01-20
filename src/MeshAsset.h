#pragma once

#include <vector>

#include "SDL_opengl.h"

struct MeshAsset {
    struct VertexData {
        float position[3];
        float normal[3];
        float colors[3];
        float uv[2];
    };
    std::vector<VertexData> vertices;
    std::vector<GLuint> indices;
};