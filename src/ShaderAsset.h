#pragma once

#include <string>

#include "GL/glew.h"
#include "SDL_opengl.h"

class ShaderAsset {
public:
    void load_shader(const std::string& vertex_path, const std::string& fragment_path);

    const bool is_valid() const;
    const GLuint get_program_id() const;

private:
    void check_error(GLuint id);

    bool valid_;
    GLuint program_id_;
};
