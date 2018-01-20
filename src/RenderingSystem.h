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

    void start_render();
    void setup_cameras();
    void endRender();

    bool init_window();
    SDL_Window* window;

    GLuint vertex_array_id_;
    Shader example_shader_;
    std::vector<RenderingComponent> example_objects_;
    float time_;
    std::array<glm::mat4x4, 4> cameras_;
    AssetManager& asset_manager_;
};

