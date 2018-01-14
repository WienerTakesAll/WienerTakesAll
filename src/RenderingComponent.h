#pragma once

#include <vector>
#include <array>

#include "GL\glew.h"
#include "SDL_opengl.h"
#include "glm\glm.hpp"

class RenderingComponent {
public:
    //Takes a list of camera matrices and renders up to 4 views
    //Does not set the shader
    void render_views(std::array<glm::mat4x4, 4>& cameras, size_t count, GLuint program_id);


    void load_from_file(const std::string& file_path);

private:
    void setupBuffer();

    struct VertexData
    {
        float position[3];
        float normal[3];
        float colors[3];
        float uv[2];
    };

    GLuint gl_vertex_buffer, gl_index_buffer;
    std::vector<VertexData> vertices;
    std::vector<GLuint> indices;
};