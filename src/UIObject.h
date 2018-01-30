#pragma once

#include "glm/glm.hpp"
#include "SDL_opengl.h"

class UIObject {
public:
    UIObject(glm::vec2 origin, glm::vec3 colour, float width, float height);
    void render() const;

    bool visible_;
    float width_;
    float height_;

private:
    glm::vec2 origin_;
    glm::vec3 colour_;
};
