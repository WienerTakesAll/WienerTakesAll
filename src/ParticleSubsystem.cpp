#include "ParticleSubsystem.h"
#include "RenderingComponent.h"
#include "AssetManager.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Powerup.h"
#include "GameState.h"

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
    , hotdog_indicator_gen_()
    , powerup_gens_()
    , powerup_pickup_gen_()
    , whos_it(0) {
}

void ParticleSubsystem::update() {
    hotdog_indicator_gen_.update();

    for (auto& powerup_gen : powerup_gens_) {
        powerup_gen.update();
    }

    powerup_pickup_gen_.update();
    powerup_pickup_gen_.set_active(false);
}

void ParticleSubsystem::render(const glm::mat4& camera, int camera_number) {
    if (camera_number != whos_it) {
        hotdog_indicator_gen_.render(camera);
    }

    for (auto& powerup_gen : powerup_gens_) {
        powerup_gen.render(camera);
    }

    powerup_pickup_gen_.render(camera);
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

    RenderingComponent pickup_component;
    pickup_component.set_mesh(mesh);
    pickup_component.set_shader(asset_manager_.get_shader_asset(PARTICLE_SHADER_PATH));
    pickup_component.set_texture(asset_manager_.get_texture_asset(POWERUP_PICKUP_PARTICLE_TEXTURE_PATH));
    powerup_pickup_gen_.init(pickup_component);
    powerup_pickup_gen_.set_active(false);
    powerup_pickup_gen_.set_probability(0.5f);
    powerup_pickup_gen_.set_particle_lifetime(120);
    powerup_pickup_gen_.set_particle_rotation_degrees(10.0f, 0.0f, 359.0f);
    powerup_pickup_gen_.set_position(glm::vec3(100.0f, 100.0f, 100.0f));
    powerup_pickup_gen_.set_particle_scale(-0.05f, 0.15f, 1.5f);
    powerup_pickup_gen_.set_particle_acceleration(glm::vec3(0.0f, -0.005f, 0.0f));
    powerup_pickup_gen_.set_spawn_amount(10, 40);
    powerup_pickup_gen_.set_spawn_range(2.0f, 1.0f, 2.0f);
    powerup_pickup_gen_.set_particle_fixed_size(false);
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

void ParticleSubsystem::handle_use_powerup(const Event& e) {
    PowerupType type = static_cast<PowerupType>(e.get_value<int>("type", true).first);
    PowerupTarget target = static_cast<PowerupTarget>(e.get_value<int>("target", true).first);
    int player_id = e.get_value<int>("index", true).first;
    glm::vec4 delta = glm::vec4(0.0f, 0.0f, 0.0f, -0.01f);
    glm::vec4 min = glm::vec4();
    glm::vec4 max = glm::vec4();

    TextureAsset* powerup_texture;
    float particle_probability = 0.0f;

    if (type == PowerupType::RELISH && target == PowerupTarget::OTHERS) {
        // special case handled by handle_dominate_controls
        return;
    }

    switch (type) {
        case PowerupType::KETCHUP:
            powerup_texture = asset_manager_.get_texture_asset(POWERUP_PARTICLE_TEXTURE_PATH);
            particle_probability = 0.15f;
            min = glm::vec4(0.4f, 0.0f, 0.0f, -0.1f);
            max = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            break;

        case PowerupType::MUSTARD:
            powerup_texture = asset_manager_.get_texture_asset(POWERUP_PARTICLE_TEXTURE_PATH);
            particle_probability = 0.15f;
            min = glm::vec4(0.4f, 0.4f, 0.0f, -0.1f);
            max = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
            break;

        case PowerupType::RELISH:
            powerup_texture = asset_manager_.get_texture_asset(POWERUP_PARTICLE_TEXTURE_PATH);
            particle_probability = 0.15f;
            min = glm::vec4(0.0f, 0.4f, 0.0f, -0.1f);
            max = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            break;

        case PowerupType::INVINCIBILITY:
            powerup_texture = asset_manager_.get_texture_asset(INVINCIBILITY_PARTICLE_TEXTURE_PATH);
            particle_probability = 1.0f;
            break;

        default:
            return;
            break;
    }

    switch (target) {
        case PowerupTarget::SELF:
            powerup_gens_[player_id].set_active(true);
            powerup_gens_[player_id].set_colour(delta, min, max);
            powerup_gens_[player_id].set_texture(powerup_texture);
            powerup_gens_[player_id].set_probability(particle_probability);
            break;

        case PowerupTarget::OTHERS:
            for (int i = 0; i < 4; ++i) {
                if (i != player_id) {
                    powerup_gens_[i].set_active(true);
                    powerup_gens_[i].set_colour(delta, min, max);
                    powerup_gens_[i].set_texture(powerup_texture);
                    powerup_gens_[i].set_probability(particle_probability);
                }
            }

            break;
    }
}

void ParticleSubsystem::handle_finish_powerup(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    powerup_gens_[object_id].set_active(false);
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

void ParticleSubsystem::handle_pickup_powerup(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    PowerupType powerup_type = static_cast<PowerupType>(e.get_value<int>("powerup_type", true).first);

    powerup_pickup_gen_.set_position(powerup_gens_[object_id].get_position());
    powerup_pickup_gen_.set_active(true);

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

    powerup_pickup_gen_.set_colour(delta, min, max);
}
