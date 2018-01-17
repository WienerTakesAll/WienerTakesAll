#include "Renderer.h"

#include "AssetManager.h"

#include <glm\gtc\matrix_transform.hpp>

Renderer::Renderer(AssetManager& n_asset_manager)
    : time(0.0f),
      asset_manager(n_asset_manager) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &Renderer::load, this);
}

void Renderer::update() {
    example_objects[1].apply_transform(glm::rotate(glm::mat4x4(), 0.01f, glm::vec3(1, 1, 1)));

    time += 0.01f;

    //This should be moved to a different system
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            std::cout << "SDL_QUIT was called" << std::endl;
            SDL_Quit();
            break;
        }
    }
}

void Renderer::load(const Event& e) {
    init_window();

    example_shader.load_shader("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");
    
    auto mesh = asset_manager.get_mesh_asset("Ship.obj");

    example_objects.emplace_back();
    example_objects[0].set_mesh(mesh);
    example_objects[0].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(0, -2, 0)));

    example_objects.emplace_back();
    example_objects[1].set_mesh(mesh);
    example_objects[1].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(1, 2, 1)));
}

void Renderer::render() {
    start_render();

    for (auto& object : example_objects) {
        object.render_views(cameras, 4, example_shader.program_id);
    }

    endRender();
}

bool Renderer::init_window() {
    SDL_Init(SDL_INIT_EVERYTHING);

    int sdl_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    int screen_width = 640;
    int screen_height = 480;

    window = SDL_CreateWindow("WienerTakesAll",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        screen_width,
        screen_height,
        sdl_flags);

    if (window == NULL) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cout << glewGetErrorString(err) << std::endl;
        return false;
    }


    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    return true;
}

void Renderer::start_render()
{
    //Setup the various camera matrices...
    setup_cameras();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glUseProgram(example_shader.program_id);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
}

void Renderer::setup_cameras()
{
    auto P = glm::perspective(glm::radians(60.f), 4.0f / 3.0f, 0.1f, 100.0f);

    cameras[0] = glm::translate(glm::mat4(), glm::vec3(5.f*std::sin(time), 5.f*std::sin(time), 5.f*std::cos(time)));
    cameras[0] = P * glm::lookAt(glm::vec3(cameras[0][3]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    cameras[1] = glm::translate(glm::mat4(), glm::vec3(5.f*std::cos(time), 5.f*std::cos(time), 5.f*std::sin(time)));
    cameras[1] = P * glm::lookAt(glm::vec3(cameras[1][3]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    cameras[2] = glm::translate(glm::mat4(), glm::vec3(5.f*std::cos(std::sqrt(time)), 5.f*std::cos(time), 5.f*std::sin(time)));
    cameras[2] = P * glm::lookAt(glm::vec3(cameras[2][3]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    cameras[3] = glm::translate(glm::mat4(), glm::vec3(5.f*std::sin(time), 5.f*std::cos(time), 5.f*std::sin(time)));
    cameras[3] = P * glm::lookAt(glm::vec3(cameras[3][3]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

void Renderer::endRender()
{
    SDL_GL_SwapWindow(window);
}