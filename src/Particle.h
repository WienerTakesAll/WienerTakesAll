#pragma once
#include <glm/vec3.hpp>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 colour;
    float rotation_rads;
    float scale;
    bool fixed_size;
    unsigned int life;
};
