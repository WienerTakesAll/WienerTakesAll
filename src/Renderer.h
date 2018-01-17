#pragma once

#include "EventSystem.h"
#include "RenderingComponent.h"
#include "Shader.h"

#include "SDL_opengl.h"
#include "SDL.h"

class AssetManager;

class Renderer : public EventSystem<Renderer>
{
public:
    Renderer(AssetManager&);
    
    void update();
    void render();
private:
    void load(const Event& e);

    void start_render();
    void setup_cameras();
    void endRender();

    bool init_window();
    SDL_Window* window;

    GLuint vertex_array_id;
    Shader example_shader;
    std::vector<RenderingComponent> example_objects;
    float time;
    std::array<glm::mat4x4, 4> cameras;
    AssetManager& asset_manager;
};

