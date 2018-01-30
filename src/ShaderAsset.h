#pragma once

#include <string>

#include "GL/glew.h"
#include "SDL_opengl.h"

class ShaderAsset {
public:
    bool load_shader(const std::string& vertex_path, const std::string& fragment_path);
	
	bool valid_;
	GLuint program_id_;

private:
    void check_error(GLuint id);
};