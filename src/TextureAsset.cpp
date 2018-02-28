#include "TextureAsset.h"

#include <iostream>

#include "SDL_image.h"

TextureAsset::TextureAsset() {
}


TextureAsset::~TextureAsset() {
}

void TextureAsset::load(const std::string& file_path) {
    SDL_Surface* surface = IMG_Load(file_path.c_str());

    if (!surface) {
        std::cerr << "Unable to load texture " << file_path << std::endl;
        valid_ = false;
    }

    //Load the texture into opengl here!
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_PixelFormat& p_format = *surface->format;

    GLint gl_p_format = GL_RGBA;
    GLint gl_p_type = GL_UNSIGNED_BYTE;
    GLsizei p_width = surface->w;
    GLsizei p_height = surface->h;

    if (p_format.BytesPerPixel == 4) {
        gl_p_format = GL_RGBA;
        gl_p_type = GL_UNSIGNED_BYTE;
    } else if (p_format.BytesPerPixel == 3) {
        gl_p_format = GL_RGB;
        gl_p_type = GL_UNSIGNED_BYTE;
    } else {
        std::cout << "Unkown pixel storage type." << std::endl;
        valid_ = false;
    }

    std::cout << "loading texture: " << file_path << std::endl;
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        gl_p_format,
        p_width,
        p_height,
        0,
        gl_p_format,
        gl_p_type,
        surface->pixels
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(surface);

    valid_ = true;
}

const bool TextureAsset::is_valid() const {
    return valid_;
}

const GLuint TextureAsset::get_texture_id() const {
    return texture_id_;
}
