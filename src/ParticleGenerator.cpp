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

#define _USE_MATH_DEFINES
#include <cmath>
#undef _USE_MATH_DEFINES

namespace {
    const float DEG_TO_RAD = M_PI / 180.0f;
}

ParticleGenerator::ParticleGenerator()
    : position_()
    , particles_()
    , rendering_component_()
    , max_particle_life_(0)
    , spawn_probability_(1.0f)
    , acceleration_()
    , scale_delta_(0.0f)
    , rotation_delta_(0.0f)
    , spawn_amount_max_(1)
    , spawn_amount_min_(1)
    , rotation_max_(0.0f)
    , rotation_min_(0.0f)
    , scale_max_(0.0f)
    , scale_min_(0.0f) {
    // TODO: replace with a different method as we are seeding the PRNG 3+ times already
    srand(time(NULL));
}

void ParticleGenerator::update() {
    // update existing particles;
    for (unsigned int index = 0; index < particles_.size(); ++index) {
        auto& particle = particles_.at(index);

        if (++particle.life > max_particle_life_) {
            particles_.erase(particles_.begin() + index);
            --index;
            continue;
        }

        particle.velocity += acceleration_;
        particle.position += particle.velocity;
        particle.rotation += rotation_delta_;
        particle.scale += scale_delta_;
    }

    generate_particles();
}

void ParticleGenerator::generate_particles() {
    bool do_spawn = ((rand() % 100) / 100.0f) < spawn_probability_;

    if (do_spawn) {
        particles_.emplace_back();
        auto& particle = particles_.back();
        particle.position = position_;
        particle.velocity = glm::vec3(0.0);
        particle.rotation_rads = generate_rotation();
        particle.scale = generate_scale();
        particle.life = 0;
    }
}

float ParticleGenerator::generate_rotation() {
    float range = rotation_max_ - rotation_min_;
    return ((rand() % 100) / 100.0f) * range + rotation_min_;
}

float ParticleGenerator::generate_scale() {
    float range = scale_max_ - scale_min_;
    return ((rand() % 100) / 100.0f) * range + scale_min_;
}

void ParticleGenerator::render(const glm::mat4& camera) {
    auto shader = rendering_component_.shader_;

    if (shader == nullptr || !shader->is_valid()) {
        std::cerr << "Trying to render with invalid shader!" << std::endl;
        return;
    }

    glUseProgram(shader->get_program_id());

    // avoid calling glm::inverse every time with a hard-coded inverse
    glm::mat4 inverse_perspective = glm::mat4( 0.769800f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.577350f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, -4.999500f,
                                    0.0f, 0.0f, -1.0f, 5.000500f);
    glm::mat4 view = inverse_perspective * camera;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindTexture(GL_TEXTURE_2D, rendering_component_.texture_->get_texture_id());
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint uniform_model = glGetUniformLocation(shader->get_program_id(), "Model");
    GLuint uniform_view = glGetUniformLocation(shader->get_program_id(), "View");

    for (auto& particle : particles_) {
        glm::mat4 model = glm::translate(glm::mat4(), particle.position);
        model *= glm::mat4(glm::transpose(glm::mat3(view)));

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

void ParticleGenerator::set_particle_acceleration(glm::vec3 acceleration) {
    acceleration_ = acceleration;
}

void ParticleGenerator::set_particle_rotation_rads(float delta, float max, float min) {
    rotation_delta_ = delta;
    rotation_max_ = max;
    rotation_min_ = min;
}

void ParticleGenerator::set_particle_rotation_degrees(float delta, float max, float min) {
    rotation_delta_ = delta * DEG_TO_RAD;
    rotation_max_ = max * DEG_TO_RAD;
    rotation_min_ = min * DEG_TO_RAD;
}

void ParticleGenerator::set_particle_scale(float delta, float max, float min) {
    scale_delta_ = delta;
    scale_max_ = max;
    scale_min_ = (min < 0 ? 0 : min);
}
