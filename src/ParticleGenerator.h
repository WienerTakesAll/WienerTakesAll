#pragma once
#include <glm/glm.hpp>
#include "RenderingComponent.h"
#include "Particle.h"

class ParticleGenerator {
public:
    ParticleGenerator();
    void update();
    void render(const glm::mat4& camera);
    bool init(RenderingComponent rendering_component);
    void set_position(glm::vec3 position);

private:
    glm::vec3 position_;
    std::vector<Particle> particles_;
    RenderingComponent rendering_component_;
    unsigned int max_particle_life_;
    float spawn_probability_;
};
