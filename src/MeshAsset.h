#pragma once

#include <vector>

#include "SDL_opengl.h"

class MeshAsset {
public:
    struct VertexData {
        float position_[3];
        float normal_[3];
        float colors_[3];
        float uv_[2];
    };
	bool valid_;
    std::vector<VertexData> vertices_;
    std::vector<GLuint> indices_;
};