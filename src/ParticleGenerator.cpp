#include "ParticleGenerator.h"
#include "RenderingComponent.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cstdlib> // for rand

ParticleGenerator::ParticleGenerator()
    : position_()
    , particles_()
    , rendering_component_()
    , max_particle_life_(0)
    , spawn_probability_(1.0) {

}

void ParticleGenerator::update() {
}

void ParticleGenerator::render(const glm::mat4& camera) {
    // set up the base matrix so that View * Model produces an identity matrix on the first three rows/columns
    glm::mat4 base_transform = {glm::mat3(camera)};
    base_transform = glm::transpose(base_transform);
    /*
    base_transform[0] = glm::vec4(camera[0][0], camera[1][0], camera[2][0], 0.0f);
    base_transform[1] = glm::vec4(camera[0][1], camera[1][1], camera[2][1], 0.0f);
    base_transform[2] = glm::vec4(camera[0][2], camera[1][2], camera[2][2], 0.0f);
    base_transform[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    */

    for (auto& particle : particles_) {
        glm::mat4 model = glm::scale(base_transform, glm::vec3(particle.scale));
        model = glm::rotate(model, particle.rotation_rads, glm::vec3(0, 0, 1));
        model = glm::translate(model, particle.position);
    }
}

bool ParticleGenerator::init(RenderingComponent rendering_component) {
    rendering_component_ = rendering_component;

    return true;
}

void ParticleGenerator::set_position(glm::vec3 position) {
    position_ = position;
}
