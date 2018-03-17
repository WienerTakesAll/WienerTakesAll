#include "ParticleSystem.h"
#include "RenderingComponent.h"
#include "AssetManager.h"

namespace {
    const std::string PARTICLE_SHADER_PATH = "assets/shaders/ParticleShader";
    const std::string HOTDOG_INDICATOR_TEXTURE_PATH = "assets/textures/default.png";
    const std::string PARTICLE_MESH_PATH = "assets/models/UIRect.obj";
}

ParticleSystem::ParticleSystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager)
    , hotdog_indicator_gen_()
    , whos_it(0) {
    /*
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &ParticleSystem::handle_load, this);
    EventSystem::add_event_handler(EventType::NEW_IT, &ParticleSystem::handle_new_it, this);
    EventSystem::add_event_handler(EventType::OBJECT_TRANSFORM_EVENT, &ParticleSystem::handle_object_transform, this);
    */
}

void ParticleSystem::update() {
    hotdog_indicator_gen_.update();
}

void ParticleSystem::render(const glm::mat4& camera) {
    hotdog_indicator_gen_.render(camera);
}

void ParticleSystem::handle_load(const Event& e) {
    MeshAsset* mesh = asset_manager_.get_mesh_asset(PARTICLE_MESH_PATH);

    // Create hotdog indicator component
    RenderingComponent indicator_component;
    indicator_component.set_mesh(mesh);
    indicator_component.set_shader(asset_manager_.get_shader_asset(PARTICLE_SHADER_PATH));
    indicator_component.set_texture(asset_manager_.get_texture_asset(HOTDOG_INDICATOR_TEXTURE_PATH));

    hotdog_indicator_gen_.init(indicator_component);
    hotdog_indicator_gen_.set_probability(1.0f);
    hotdog_indicator_gen_.set_particle_lifetime(1);
}

void ParticleSystem::handle_new_it(const Event& e) {
    whos_it = e.get_value<int>("object_id", true).first;
}

void ParticleSystem::handle_object_transform(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    if (object_id == whos_it) {
        // update hotdog indicator
        int pos_x = e.get_value<float>("pos_x", true).first;
        int pos_y = e.get_value<float>("pos_y", true).first;
        int pos_z = e.get_value<float>("pos_z", true).first;

        hotdog_indicator_gen_.set_position(glm::vec3(pos_x, pos_y + 1, pos_z));
    }
}
