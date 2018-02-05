#pragma once

#include "EventSystem.h"
#include "RenderingComponent.h"
#include "Shader.h"

#include "SDL_opengl.h"
#include "SDL.h"

class AssetManager;

class RenderingSystem : public EventSystem<RenderingSystem> {
public:
    RenderingSystem(AssetManager&);

    void update();
    void render();

private:
    void load(const Event& e);
    void handle_key_press(const Event& e);
    void handle_add_game_object(const Event& e);

    void start_render() const;
    void setup_cameras();
    void end_render() const;

    bool init_window();

    SDL_Window* window_;
    GLuint vertex_array_id_;
    Shader example_shader_;
    std::vector<RenderingComponent> example_objects_;
    std::array<glm::mat4x4, 4> cameras_;
    AssetManager& asset_manager_;
};

