#pragma once

#include <vector>

#include "GL/glew.h"
#include "SDL_opengl.h"
#include "glm/glm.hpp"


struct MeshAsset {
    struct MeshData {
        struct VertexData {
            glm::vec3 position_;
            glm::vec3 normal_;
            glm::vec3 colors_;
            glm::vec3 uv_;
        };
        std::vector<VertexData> vertices_;
        std::vector<GLuint> indices_;

        struct VolumeVertexData {
            glm::vec4 position_;
            glm::vec3 normal_;
        };

        std::vector<VolumeVertexData> shadow_volume_vertices_;
        std::vector<GLuint> shadow_volume_indices_;

    };
    bool valid_;
    std::vector<MeshData> meshes_;
};
