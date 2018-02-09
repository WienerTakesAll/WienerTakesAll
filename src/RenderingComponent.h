#pragma once

#include <vector>
#include <array>

#include "GL/glew.h"
#include "SDL_opengl.h"
#include "glm/glm.hpp"

#include "MeshAsset.h"

class RenderingComponent {
public:
    //Takes a list of camera matrices and renders up to 4 views
    //Does not set the shader
    void render_views(std::array<glm::mat4x4, 4>& cameras, size_t count, GLuint program_id);

    void apply_transform(glm::mat4x4 transform);
    void set_transform(glm::mat4x4 transform);
    const glm::mat4x4& get_transform() const;

    void set_mesh(MeshAsset* mesh);

private:
    void setupBuffer();

    std::vector<GLuint> gl_vertex_buffers_;
    std::vector<GLuint> gl_index_buffers_;
    glm::mat4 transform_matrix_;
    MeshAsset* mesh_;
};