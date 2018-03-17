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
    // void set_gravity(float gravity);
    void set_probability(const float& probability);
    void set_particle_lifetime(const unsigned int& lifetime);
    // void set_spawn_amount(int max, int min);
    // void set_particle_velocity();
    // void set_particle_rotation(float delta, float max, float min);
    // void set_particle_scale(float max, float min);

private:
    void generate_particles();
    void generate_velocity();
    void generate_rotation();
    void generate_scale();

    glm::vec3 position_;
    std::vector<Particle> particles_;
    RenderingComponent rendering_component_;
    unsigned int max_particle_life_;
    float spawn_probability_;
    /*
    float gravity_;
    float scale_delta;
    float rotation_delta;

    int spawn_amount_max_;
    int spawn_amount_min_;
    float rotation_max_;
    float rotation_min_;
    float scale_max_;
    float scale_min_;
    */
};
