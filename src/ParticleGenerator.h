#pragma once
#include <glm/glm.hpp>
#include "RenderingComponent.h"
#include "Particle.h"

class ParticleGenerator {
public:
    ParticleGenerator();
    void update();
    void render(const glm::mat4& camera) const;
    bool init(RenderingComponent rendering_component);
    void set_active(const bool& active);
    void set_position(glm::vec3 position);
    void set_probability(const float& probability);
    void set_particle_lifetime(const unsigned int& lifetime);
    void set_spawn_amount(int min, int max);
    void set_spawn_range(float x, float y, float z);
    void set_particle_acceleration(glm::vec3 acceleration);
    void set_particle_rotation_rads(float delta, float min, float max);
    void set_particle_rotation_degrees(float delta, float min, float max);
    void set_particle_scale(float delta, float min, float max);
    void set_particle_fixed_size(bool is_fixed_size);
    void set_colour(glm::vec4 delta, glm::vec4 min, glm::vec4 max);

private:
    bool active_;
    void generate_particles(const bool& force = false);
    glm::vec3 generate_position() const;
    float generate_rotation() const;
    float generate_scale() const;
    glm::vec4 generate_colour() const;

    glm::vec3 position_;
    std::vector<Particle> particles_;
    RenderingComponent rendering_component_;
    unsigned int max_particle_life_;
    float spawn_probability_;
    glm::vec3 acceleration_;
    float scale_delta_;
    float rotation_delta_;

    int spawn_amount_max_;
    int spawn_amount_min_;
    glm::vec3 spawn_distance_;
    float rotation_max_;
    float rotation_min_;
    float scale_max_;
    float scale_min_;
    glm::vec4 colour_change_;
    glm::vec4 colour_min_;
    glm::vec4 colour_max_;

    bool fixed_size_;
};
