#pragma once

#include <string>

#include "GL/glew.h"
#include "SDL_opengl.h"

class Shader
{
public:
    bool load_shader(const std::string& vertex_path, const std::string& fragment_path);

    GLuint program_id;
private:
    void check_error(GLuint id);
};