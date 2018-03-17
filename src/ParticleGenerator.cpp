#include "ParticleGenerator.h"
#include "RenderingComponent.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"
#include "MeshAsset.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>
#include <iostream>
#include <cstdlib> // for rand
#include <ctime>

ParticleGenerator::ParticleGenerator()
    : position_()
    , particles_()
    , rendering_component_()
    , max_particle_life_(0)
    , spawn_probability_(1.0) {
    // TODO: replace with a different method as we are seeding the PRNG 3+ times already
    srand(time(NULL));
}

void ParticleGenerator::update() {
    // update existing particles;
    for (unsigned int index = 0; index < particles_.size(); ++index) {
        auto& particle = particles_.at(index);

        ++particle.life;

        if (particle.life > max_particle_life_) {
            particles_.erase(particles_.begin() + index);
            --index;
            continue;
        }

        // particle.velocity += glm::vec3(gravity_);
        particle.position += particle.velocity;
    }

    generate_particles();
}

void ParticleGenerator::generate_particles() {
    bool do_spawn = ((rand() % 100) / 100.0f) < spawn_probability_;

    if (do_spawn) {
        particles_.emplace_back();
        auto& particle = particles_.back();
        particle.position = position_;
        particle.velocity = glm::vec3(0.0); //generate_velocty();
        particle.rotation_rads = 0.0; //generate_rotation();
        particle.scale = 1.0; //generate_scale();
        particle.life = 0;
    }
}

void ParticleGenerator::render(const glm::mat4& camera) {
    auto shader = rendering_component_.shader_;

    if (shader == nullptr || !shader->is_valid()) {
        std::cerr << "Trying to render with invalid shader!" << std::endl;
        return;
    }

    glUseProgram(shader->get_program_id());

    // set up the base matrix so that View * Model produces an identity matrix on the first three rows/columns
    glm::mat4 base_transform = {glm::mat3(camera)};
    base_transform = glm::transpose(base_transform);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindTexture(GL_TEXTURE_2D, rendering_component_.texture_->get_texture_id());
    glEnable(GL_TEXTURE_2D);

    GLuint uniform_model = glGetUniformLocation(shader->get_program_id(), "Model");
    GLuint uniform_view = glGetUniformLocation(shader->get_program_id(), "View");

    for (auto& particle : particles_) {
        glm::mat4 model = glm::scale(base_transform, glm::vec3(particle.scale));
        model = glm::rotate(model, particle.rotation_rads, glm::vec3(0, 0, 1));
        model = glm::translate(model, particle.position);

        for (size_t i = 0; i < rendering_component_.mesh_->meshes_.size(); ++i) {
            glBindBuffer(GL_ARRAY_BUFFER, rendering_component_.gl_vertex_buffers_[i]);
            glVertexAttribPointer
            (0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VertexData)
             , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VertexData, position_)));
            glVertexAttribPointer
            (1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VertexData)
             , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VertexData, normal_)));
            glVertexAttribPointer
            (2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VertexData)
             , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VertexData, uv_)));

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendering_component_.gl_index_buffers_[i]);


            glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(camera));

            glDrawElements(GL_TRIANGLES, rendering_component_.mesh_->meshes_[i].indices_.size(), GL_UNSIGNED_INT, 0);
        }
    }
}

bool ParticleGenerator::init(RenderingComponent rendering_component) {
    rendering_component_ = rendering_component;

    return true;
}

void ParticleGenerator::set_position(glm::vec3 position) {
    position_ = position;
}

void ParticleGenerator::set_probability(const float& probability) {
    spawn_probability_ = probability;
}

void ParticleGenerator::set_particle_lifetime(const unsigned int& lifetime) {
    max_particle_life_ = lifetime;
}

/*
void ParticleGenerator::set_gravity(float gravity) {
}
*/
