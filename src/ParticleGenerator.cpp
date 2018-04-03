#define _USE_MATH_DEFINES
#include <cmath>
#undef _USE_MATH_DEFINES

#include "ParticleGenerator.h"
#include "RenderingComponent.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"
#include "MeshAsset.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/type_ptr.hpp"
#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>
#include <cstdlib> // for rand
#include <ctime>

namespace {
    const float DEG_TO_RAD = M_PI / 180.0f;
}

ParticleGenerator::ParticleGenerator()
    : active_(true)
    , position_()
    , particles_()
    , rendering_component_()
    , max_particle_life_(0)
    , spawn_probability_(1.0f)
    , acceleration_()
    , scale_delta_(0.0f)
    , rotation_delta_(0.0f)
    , spawn_amount_max_(1)
    , spawn_amount_min_(1)
    , spawn_distance_()
    , rotation_max_(0.0f)
    , rotation_min_(0.0f)
    , scale_max_(1.0f)
    , scale_min_(1.0f) {
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
        particle.rotation_rads += rotation_delta_;
        particle.scale = glm::clamp(particle.scale + scale_delta_, scale_min_, scale_max_);
        particle.colour = glm::clamp(particle.colour + colour_change_, glm::vec4(-1.0f), glm::vec4(1.0f));
    }

    if (!active_) {
        return;
    }

    for (int i = 0; i < spawn_amount_min_; ++i) {
        generate_particles(true);
    }

    for (int i = 0; i < spawn_amount_max_ - spawn_amount_min_; ++i) {
        generate_particles();
    }
}

void ParticleGenerator::generate_particles(const bool& force) {
    bool do_spawn = ((rand() % 1000) / 1000.0f) < spawn_probability_;

    if (do_spawn || force) {
        particles_.emplace_back();
        auto& particle = particles_.back();
        particle.position = generate_position();
        particle.velocity = glm::vec3(0.0);
        particle.rotation_rads = generate_rotation();
        particle.scale = generate_scale();
        particle.fixed_size = false;
        particle.life = 0;
        particle.colour = generate_colour();
    }
}

glm::vec3 ParticleGenerator::generate_position() const {
    float x_factor = ((rand() % 100) / 50.0f) - 1.0f;
    float y_factor = ((rand() % 100) / 50.0f) - 1.0f;
    float z_factor = ((rand() % 100) / 50.0f) - 1.0f;
    return position_ + glm::vec3(spawn_distance_[0] * x_factor, spawn_distance_[1] * y_factor, spawn_distance_[2] * z_factor);
}

float ParticleGenerator::generate_rotation() const {
    float range = rotation_max_ - rotation_min_;
    return ((rand() % 100) / 100.0f) * range + rotation_min_;
}

float ParticleGenerator::generate_scale() const {
    float range = scale_max_ - scale_min_;
    return ((rand() % 100) / 100.0f) * range + scale_min_;
}

glm::vec4 ParticleGenerator::generate_colour() const {
    glm::vec4 range = colour_max_ - colour_min_;
    float r = ((rand() % 100) / 100.0f) * range[0] + colour_min_[0];
    float g = ((rand() % 100) / 100.0f) * range[1] + colour_min_[1];
    float b = ((rand() % 100) / 100.0f) * range[2] + colour_min_[2];
    float a = ((rand() % 100) / 100.0f) * range[3] + colour_min_[3];

    return glm::vec4(r, g, b, a);
}

void ParticleGenerator::render(const glm::mat4& camera) const {
    if (particles_.empty()) {
        return;
    }

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
    GLuint uniform_overlay = glGetUniformLocation(shader->get_program_id(), "Overlay");

    for (auto& particle : particles_) {
        glm::mat4 model = glm::translate(glm::mat4(), particle.position);
        model *= glm::mat4(glm::transpose(glm::mat3(view)));
        model = glm::scale(model, glm::vec3(particle.scale));
        model = glm::rotate(model, particle.rotation_rads, glm::vec3(0.0f, 0.0f, 1.0f));

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


            if (fixed_size_) {
                glm::vec4 topLeftVertex = glm::vec4(rendering_component_.mesh_->meshes_[i].vertices_[2].position_, 1.0f);
                glm::vec4 bottomRightVertex = glm::vec4(rendering_component_.mesh_->meshes_[i].vertices_[0].position_, 1.0f);

                topLeftVertex = camera * model * topLeftVertex;
                bottomRightVertex = camera * model * bottomRightVertex;

                float scale_value = std::sqrt(bottomRightVertex.z);

                model = glm::scale(model, glm::vec3(scale_value, scale_value, 1.f));
                model = glm::translate(model, glm::vec3(0.f, -0.15f * scale_value, 0.f));
            }


            glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(camera));
            glUniform4fv(uniform_overlay, 1, glm::value_ptr(particle.colour));

            glDrawElements(GL_TRIANGLES, rendering_component_.mesh_->meshes_[i].indices_.size(), GL_UNSIGNED_INT, 0);
        }
    }
}

bool ParticleGenerator::init(RenderingComponent rendering_component) {
    rendering_component_ = rendering_component;

    return true;
}

void ParticleGenerator::clear() {
    particles_.clear();
}

void ParticleGenerator::set_active(const bool& active) {
    active_ = active;
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

void ParticleGenerator::set_spawn_amount(int min, int max) {
    spawn_amount_max_ = max;
    spawn_amount_min_ = min;
}

void ParticleGenerator::set_spawn_range(float x, float y, float z) {
    spawn_distance_ = glm::vec3(x, y, z);
}

void ParticleGenerator::set_particle_acceleration(glm::vec3 acceleration) {
    acceleration_ = acceleration;
}

void ParticleGenerator::set_particle_rotation_rads(float delta, float min, float max) {
    rotation_delta_ = delta;
    rotation_max_ = max;
    rotation_min_ = min;
}

void ParticleGenerator::set_particle_rotation_degrees(float delta, float min, float max) {
    rotation_delta_ = delta * DEG_TO_RAD;
    rotation_max_ = max * DEG_TO_RAD;
    rotation_min_ = min * DEG_TO_RAD;
}

void ParticleGenerator::set_particle_scale(float delta, float min, float max) {
    scale_delta_ = delta;
    scale_max_ = max;
    scale_min_ = (min < 0 ? 0 : min);
}

void ParticleGenerator::set_particle_fixed_size(bool is_fixed_size) {
    fixed_size_ = is_fixed_size;
}

void ParticleGenerator::set_colour(glm::vec4 delta, glm::vec4 min, glm::vec4 max) {
    colour_change_ = delta;
    colour_max_ = max;
    colour_min_ = min;
}

void ParticleGenerator::set_texture(TextureAsset* texture) {
    rendering_component_.set_texture(texture);
}
