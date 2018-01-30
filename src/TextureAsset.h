#pragma once

#include <string>

#include "GL/glew.h"
#include "SDL_opengl.h"

class TextureAsset
{
public:
	TextureAsset();
	~TextureAsset();
	bool load_texture(const std::string& file_path);

	bool valid_;
	GLuint texture_id_;
};

