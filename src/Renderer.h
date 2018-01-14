#pragma once

#include "EventSystem.h"
#include "RenderingComponent.h"
#include "Shader.h"

#include "SDL_opengl.h"
#include "SDL.h"

class Renderer : public EventSystem<Renderer>
{
public:
    Renderer();
    
    void update();
    void render();
private:
    void load(Event e);

    void start_render();
    void setup_cameras();
    void endRender();

    bool init_window();
    SDL_Window* window;

    GLuint vertex_array_id;
    Shader example_shader;
    RenderingComponent example_object;
    float time;
    std::array<glm::mat4x4, 4> cameras;
};

