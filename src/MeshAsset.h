#pragma once

#include <vector>

#include "GL/glew.h"
#include "SDL_opengl.h"
#include "glm/glm.hpp"

class MeshAsset {
public:
    struct VertexData {
        glm::vec3 position_;
        glm::vec3 normal_;
        glm::vec3 colors_;
        glm::vec2 uv_;
    };
    bool valid_;
    std::vector<VertexData> vertices_;
    std::vector<GLuint> indices_;
};
