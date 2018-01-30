#include "TextureAsset.h"

#include <iostream>

#include "SDL_image.h"

TextureAsset::TextureAsset() {
}


TextureAsset::~TextureAsset() {
}

bool TextureAsset::load_texture(const std::string& file_path) {
	SDL_Surface* surface = IMG_Load(file_path.c_str());

	if (!surface) {
		std::cerr << "Unable to load texture " << file_path << std::endl;
		return false;
	}

	//Load the texture into opengl here!
	glGenTextures(1, &texture_id_);
	glBindTexture(GL_TEXTURE_2D, texture_id_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_PixelFormat& p_format = *surface->format;

	GLint gl_p_format;
	GLint gl_p_type;
	GLsizei p_width = surface->w;
	GLsizei p_height = surface->h;

	if (p_format.BytesPerPixel == 4) {
		gl_p_format = GL_RGBA;
		gl_p_type = GL_UNSIGNED_INT;
	} else if (p_format.BytesPerPixel == 3) {
		gl_p_format = GL_RGB;
		gl_p_type = GL_UNSIGNED_BYTE;
	} else {
		std::cout << "Unkown pixel storage type." << std::endl;
		return false;
	}



	glTexImage2D
		( GL_TEXTURE_2D, 0, GL_RGB, p_width, p_height
		, 0, gl_p_format, gl_p_type, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(surface);

	return true;
}