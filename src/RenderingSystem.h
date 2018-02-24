#pragma once

#include "EventSystem.h"
#include "RenderingComponent.h"
#include "ShaderAsset.h"

#include "GL/glew.h"
#include "SDL_opengl.h"
#include "SDL.h"

#include "TextureAsset.h"

class AssetManager;

class RenderingSystem : public EventSystem<RenderingSystem> {
public:
    RenderingSystem(AssetManager&);

    void update();
    void render();

private:
    void load(const Event& e);
    void handle_key_press(const Event& e);
    void handle_add_vehicle(const Event& e);
    void handle_add_terrain(const Event& e);
    void handle_object_transform(const Event& e);

    void start_render() const;
    void setup_cameras();
    void end_render() const;

    bool init_window();

    SDL_Window* window_;
    GLuint vertex_array_id_;
    ShaderAsset example_shader_;
    std::vector<RenderingComponent> example_objects_;
    std::vector<size_t> car_indices_;
    std::array<glm::mat4x4, 4> cameras_;
    AssetManager& asset_manager_;
};

