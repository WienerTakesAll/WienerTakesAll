#pragma once

#include "glm/glm.hpp"

#include "GL/glew.h"
#include "SDL_opengl.h"

#include "RenderingComponent.h"

class UIObject {
public:
    UIObject() = default;
    UIObject(
        glm::vec2 origin,
        glm::vec3 colour,
        glm::vec2 size,
        MeshAsset* mesh,
        TextureAsset* tex,
        ShaderAsset* shader
    );
    void set_origin(glm::vec2 origin);
    void render(glm::mat4 camera) const;
    void set_texture(TextureAsset* tex);
    void set_scale(float factor);
    void set_rotation(float angle_radians);

    bool visible_;

private:
    glm::vec2 origin_;
    glm::vec3 colour_;
    glm::vec2 size_;
    float scale_;
    RenderingComponent render_component_;
};
