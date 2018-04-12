#pragma once
#include "EventSystem.h"
#include <glm/glm.hpp>
#include "ParticleGenerator.h"
#include <array>

class AssetManager;

class ParticleSubsystem {
public:
    ParticleSubsystem(AssetManager& asset_manager);
    void update();
    void render(const glm::mat4& camera, int camera_num);
    void handle_load(const Event& e);
    void handle_new_it(const Event& e);
    void handle_object_transform(const Event& e);
    void handle_new_status_effect(const Event& e);
    void handle_new_game_state(const Event& e);
    void handle_pickup_powerup(const Event& e);

private:
    AssetManager& asset_manager_;
    ParticleGenerator hotdog_indicator_gen_;
    std::array<ParticleGenerator, 4> powerup_gens_;
    ParticleGenerator powerup_pickup_gen_;

    int whos_it;
};
