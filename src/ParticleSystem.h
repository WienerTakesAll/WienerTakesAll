#pragma once
#include "EventSystem.h"
#include <glm/glm.hpp>
#include "ParticleGenerator.h"

class AssetManager;

class ParticleSystem : public EventSystem<ParticleSystem> {
public:
    ParticleSystem(AssetManager& asset_manager);
    void update();
    void render(const glm::mat4& camera, int camera_num);
    void handle_load(const Event& e);
    void handle_new_it(const Event& e);
    void handle_object_transform(const Event& e);

private:

    AssetManager& asset_manager_;
    ParticleGenerator hotdog_indicator_gen_;

    int whos_it;
};
