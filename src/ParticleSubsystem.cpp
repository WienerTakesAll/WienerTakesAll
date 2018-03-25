#include "ParticleSubsystem.h"
#include "RenderingComponent.h"
#include "AssetManager.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {
    const std::string PARTICLE_SHADER_PATH = "assets/shaders/ParticleShader";
    const std::string HOTDOG_INDICATOR_TEXTURE_PATH = "assets/textures/pointer.png";
    const std::string PARTICLE_MESH_PATH = "assets/models/UIRect.obj";
}

ParticleSubsystem::ParticleSubsystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager)
    , hotdog_indicator_gen_()
    , whos_it(0) {
}

void ParticleSubsystem::update() {
    hotdog_indicator_gen_.update();
}

void ParticleSubsystem::render(const glm::mat4& camera, int camera_number) {
    if (camera_number != whos_it) {
        hotdog_indicator_gen_.render(camera);
    }
}

void ParticleSubsystem::handle_load(const Event& e) {
    MeshAsset* mesh = asset_manager_.get_mesh_asset(PARTICLE_MESH_PATH);

    // Create hotdog indicator component
    RenderingComponent indicator_component;
    indicator_component.set_mesh(mesh);
    indicator_component.set_shader(asset_manager_.get_shader_asset(PARTICLE_SHADER_PATH));
    indicator_component.set_texture(asset_manager_.get_texture_asset(HOTDOG_INDICATOR_TEXTURE_PATH));

    hotdog_indicator_gen_.init(indicator_component);
    hotdog_indicator_gen_.set_probability(1.0f);
    hotdog_indicator_gen_.set_particle_lifetime(0);
    hotdog_indicator_gen_.set_particle_rotation_degrees(0.0f, 180.0f, 180.0f);
    hotdog_indicator_gen_.set_position(glm::vec3(100.0f, 100.0f, 100.0f));
}

void ParticleSubsystem::handle_new_it(const Event& e) {
    whos_it = e.get_value<int>("object_id", true).first;
}

void ParticleSubsystem::handle_object_transform(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    if (object_id == whos_it) {
        // update hotdog indicator
        float pos_x = e.get_value<float>("pos_x", true).first;
        float pos_y = e.get_value<float>("pos_y", true).first;
        float pos_z = e.get_value<float>("pos_z", true).first;

        float qw = e.get_value<float>("qua_w", true).first;
        float qx = e.get_value<float>("qua_x", true).first;
        float qy = e.get_value<float>("qua_y", true).first;
        float qz = e.get_value<float>("qua_z", true).first;

        // try to center the indicator
        glm::mat4 rotation = glm::toMat4(glm::quat(qx, qy, qz, qw));
        glm::vec3 right = glm::normalize(glm::vec3(rotation[0]));
        glm::vec3 back = glm::normalize(glm::vec3(rotation[2]));
        hotdog_indicator_gen_.set_position(glm::vec3(pos_x, pos_y + 3, pos_z) + (0.5f * right) - back);
    }
}
