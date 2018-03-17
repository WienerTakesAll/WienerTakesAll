#pragma once
#include <glm/vec3.hpp>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float rotation_rads;
    float scale;
    unsigned int life;
};
