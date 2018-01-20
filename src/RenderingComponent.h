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

    void set_mesh(MeshAsset* n_mesh);

private:
    void setupBuffer();

    GLuint gl_vertex_buffer, gl_index_buffer_;
    glm::mat4 transform_matrix_;
    MeshAsset* mesh_;
};