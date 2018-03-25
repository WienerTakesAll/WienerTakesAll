#include "TextureAsset.h"

#include <iostream>

#include "SDL_image.h"

TextureAsset::TextureAsset() {
}


TextureAsset::~TextureAsset() {
}

void TextureAsset::load(const std::string& file_path, const bool& do_clamp) {
    SDL_Surface* surface = IMG_Load(file_path.c_str());

    if (!surface) {
        std::cerr << "Unable to load texture " << file_path << std::endl;
        valid_ = false;
        return;
    }

    //Load the texture into opengl here!
    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (do_clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }

    SDL_PixelFormat& p_format = *surface->format;

    GLint gl_p_format = GL_RGBA;
    GLint gl_p_type = GL_UNSIGNED_BYTE;
    GLsizei p_width = surface->w;
    GLsizei p_height = surface->h;

    if (p_format.BytesPerPixel == 4) {

#ifdef __APPLE__

        for (size_t i = 0; i < p_width * p_height; i++) {
            int pix = ((int*)(surface->pixels))[i];
            ((int*)(surface->pixels))[i] = ((pix & 0xFF) << 16) | (pix & 0xFF00) | ((pix & 0xFF0000) >> 16) | (pix & 0xFF000000);
        }

#endif



        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
