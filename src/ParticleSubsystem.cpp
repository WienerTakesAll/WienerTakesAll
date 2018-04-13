#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ParticleSubsystem.h"
#include "RenderingComponent.h"
#include "AssetManager.h"
#include "Powerup.h"
#include "GameState.h"
#include "StatusEffect.h"

namespace {
    const std::string PARTICLE_SHADER_PATH = "assets/shaders/ParticleShader";
    const std::string HOTDOG_INDICATOR_TEXTURE_PATH = "assets/textures/pointer.png";
    const std::string PARTICLE_MESH_PATH = "assets/models/UIRect.obj";
    const std::string POWERUP_PARTICLE_TEXTURE_PATH = "assets/textures/powerup_drop.png";
    const std::string INVINCIBILITY_PARTICLE_TEXTURE_PATH = "assets/textures/stardim.png";
    const std::string POWERUP_PICKUP_PARTICLE_TEXTURE_PATH = "assets/textures/pickup.png";
}

ParticleSubsystem::ParticleSubsystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager)
    , whos_it(0) {
}

void ParticleSubsystem::update() {
    hotdog_indicator_gen_.update();

    for (auto& powerup_gen : powerup_gens_) {
        powerup_gen.update();
    }

    for (auto& powerup_pickup_gen : powerup_pickup_gens_) {
        powerup_pickup_gen.second.update();
        powerup_pickup_gen.second.set_active(false);
    }
}

void ParticleSubsystem::render(const glm::mat4& camera, int camera_number) {
    if (camera_number != whos_it) {
        hotdog_indicator_gen_.render(camera);
    }

    for (auto& powerup_gen : powerup_gens_) {
        powerup_gen.render(camera);
    }

    for (auto& powerup_pickup_gen : powerup_pickup_gens_) {
        powerup_pickup_gen.second.render(camera);
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
    hotdog_indicator_gen_.set_particle_fixed_size(true);

    RenderingComponent powerup_component;
    powerup_component.set_mesh(mesh);
    powerup_component.set_shader(asset_manager_.get_shader_asset(PARTICLE_SHADER_PATH));
    powerup_component.set_texture(asset_manager_.get_texture_asset(POWERUP_PARTICLE_TEXTURE_PATH));

    for (auto& powerup_gen : powerup_gens_) {
        powerup_gen.init(powerup_component);
        powerup_gen.set_active(false);
        powerup_gen.set_probability(0.15f);
        powerup_gen.set_particle_lifetime(120);
        powerup_gen.set_particle_rotation_degrees(5.0f, 0.0f, 360.0f);
        powerup_gen.set_position(glm::vec3(100.0f, 100.0f, 100.0f));
        powerup_gen.set_particle_scale(-0.01f, 0.0f, 0.9f);
        powerup_gen.set_particle_acceleration(glm::vec3(0.0f, -0.01f, 0.0f));
        powerup_gen.set_spawn_amount(0, 1);
        powerup_gen.set_spawn_range(2.0f, 1.0f, 2.0f);
    }
}

void ParticleSubsystem::handle_new_it(const Event& e) {
    whos_it = e.get_value<int>("object_id", true).first;
}

void ParticleSubsystem::handle_object_transform(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    float pos_x = e.get_value<float>("pos_x", true).first;
    float pos_y = e.get_value<float>("pos_y", true).first;
    float pos_z = e.get_value<float>("pos_z", true).first;

    if (object_id < 4 ) {
        powerup_gens_[object_id].set_position(glm::vec3(pos_x, pos_y + 1.0f, pos_z));
    }

    if (object_id == whos_it) {
        // update hotdog indicator

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

void ParticleSubsystem::handle_new_status_effect(const Event& e) {
    StatusEffect type = static_cast<StatusEffect>(e.get_value<int>("type", true).first);
    int player_id = e.get_value<int>("object_id", true).first;

    TextureAsset* powerup_texture = asset_manager_.get_texture_asset(POWERUP_PARTICLE_TEXTURE_PATH);
    float particle_probability = 0.0f;
    glm::vec4 delta = glm::vec4(0.0f, 0.0f, 0.0f, -0.01f);
    glm::vec4 min = glm::vec4();
    glm::vec4 max = glm::vec4();
    bool new_active = true;

    switch (type) {
        case StatusEffect::BAD_KETCHUP:
            new_active = true;
            powerup_texture = asset_manager_.get_texture_asset(POWERUP_PARTICLE_TEXTURE_PATH);
            particle_probability = 0.15f;
            min = glm::vec4(0.4f, 0.0f, 0.0f, -0.1f);
            max = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            break;

        case StatusEffect::GOOD_KETCHUP:
            new_active = true;
            powerup_texture = asset_manager_.get_texture_asset(POWERUP_PARTICLE_TEXTURE_PATH);
            particle_probability = 0.15f;
            min = glm::vec4(0.4f, 0.0f, 0.0f, -0.1f);
            max = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            break;

        case StatusEffect::MUSTARD_EFFECT:
            new_active = true;
            powerup_texture = asset_manager_.get_texture_asset(POWERUP_PARTICLE_TEXTURE_PATH);
            particle_probability = 0.15f;
            min = glm::vec4(0.4f, 0.4f, 0.0f, -0.1f);
            max = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
            break;

        case StatusEffect::INVINCIBILITY:
            new_active = true;
            powerup_texture = asset_manager_.get_texture_asset(INVINCIBILITY_PARTICLE_TEXTURE_PATH);
            particle_probability = 1.0f;
            break;

        case StatusEffect::CONTROLS_REVERSED:
            new_active = true;
            powerup_texture = asset_manager_.get_texture_asset(POWERUP_PARTICLE_TEXTURE_PATH);
            particle_probability = 0.15f;
            min = glm::vec4(0.0f, 0.4f, 0.0f, -0.1f);
            max = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            break;

        case StatusEffect::DOMINATED:
        case StatusEffect::NONE:
            new_active = false;
            break;

        default:
            assert(false);
            break;
    }

    powerup_gens_[player_id].set_active(new_active);
    powerup_gens_[player_id].set_colour(delta, min, max);
    powerup_gens_[player_id].set_texture(powerup_texture);
    powerup_gens_[player_id].set_probability(particle_probability);
}

void ParticleSubsystem::handle_new_game_state(const Event& e) {
    GameState new_game_state = (GameState)e.get_value<int>("state", true).first;

    if (new_game_state == GameState::END_GAME || new_game_state == GameState::START_MENU) {
        hotdog_indicator_gen_.clear();
        hotdog_indicator_gen_.set_active(false);

        for (auto& powerup_gen : powerup_gens_) {
            powerup_gen.clear();
            powerup_gen.set_active(false);
        }
    } else if (new_game_state == GameState::IN_GAME) {
        hotdog_indicator_gen_.set_active(true);
    }
}

void ParticleSubsystem::handle_add_powerup(const Event& e) {
    int powerup_id = e.get_value<int>("object_id", true).first;
    powerup_pickup_gens_[powerup_id] = ParticleGenerator();

    RenderingComponent pickup_component;
    pickup_component.set_mesh(asset_manager_.get_mesh_asset(PARTICLE_MESH_PATH));
    pickup_component.set_shader(asset_manager_.get_shader_asset(PARTICLE_SHADER_PATH));
    pickup_component.set_texture(asset_manager_.get_texture_asset(POWERUP_PICKUP_PARTICLE_TEXTURE_PATH));
    powerup_pickup_gens_[powerup_id].init(pickup_component);
    powerup_pickup_gens_[powerup_id].set_active(false);
    powerup_pickup_gens_[powerup_id].set_probability(0.5f);
    powerup_pickup_gens_[powerup_id].set_particle_lifetime(120);
    powerup_pickup_gens_[powerup_id].set_particle_rotation_degrees(10.0f, 0.0f, 359.0f);
    powerup_pickup_gens_[powerup_id].set_position(glm::vec3(100.0f, 100.0f, 100.0f));
    powerup_pickup_gens_[powerup_id].set_particle_scale(-0.05f, 0.15f, 1.5f);
    powerup_pickup_gens_[powerup_id].set_particle_acceleration(glm::vec3(0.0f, -0.005f, 0.0f));
    powerup_pickup_gens_[powerup_id].set_spawn_amount(10, 40);
    powerup_pickup_gens_[powerup_id].set_spawn_range(2.0f, 1.0f, 2.0f);
    powerup_pickup_gens_[powerup_id].set_particle_fixed_size(false);
}

void ParticleSubsystem::handle_pickup_powerup(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    int powerup_id = e.get_value<int>("powerup_id", true).first;
    PowerupType powerup_type = static_cast<PowerupType>(e.get_value<int>("powerup_type", true).first);

    powerup_pickup_gens_[powerup_id].set_position(powerup_gens_[object_id].get_position());
    powerup_pickup_gens_[powerup_id].set_active(true);

    glm::vec4 delta(0.0f, 0.0f, 0.0f, -0.05f);
    glm::vec4 min(0.0f, 0.0f, 0.0f, 0.0f);
    glm::vec4 max(1.0f, 1.0f, 1.0f, 0.0f);

    if (powerup_type == PowerupType::KETCHUP) {
        delta[0] = 0.3f;
        delta[1] = 0.10f;
        delta[2] = 0.10f;
        min[0] = 0.6f;
    } else if (powerup_type == PowerupType::MUSTARD) {
        delta[0] = 0.3f;
        delta[1] = 0.3f;
        delta[2] = 0.05f;
        min[0] = 0.6f;
        min[1] = 0.6f;
    } else if (powerup_type == PowerupType::RELISH) {
        delta[0] = 0.10f;
        delta[1] = 0.3f;
        delta[2] = 0.10f;
        min[1] = 0.6f;
    }

    powerup_pickup_gens_[powerup_id].set_colour(delta, min, max);
}
