#pragma once

#include <string>

#include "GL/glew.h"
#include "SDL_opengl.h"

class TextureAsset {
public:
    TextureAsset();
    ~TextureAsset();
    void load(const std::string& file_path, const bool& do_clamp = false);
    const bool is_valid() const;
    const GLuint get_texture_id() const;

private:
    bool valid_;
    GLuint texture_id_;
};

