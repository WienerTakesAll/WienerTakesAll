#include "RenderingSystem.h"

#include "AssetManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace {
    const std::string SHIP_VERTEX_SHADER_PATH = "assets/shaders/SimpleVertexShader.vertexshader";
    const std::string SHIP_FRAGMENT_SHADER_PATH = "assets/shaders/SimpleFragmentShader.fragmentshader";
    const std::string SHIP_MESH_PATH = "assets/models/carBoxModel.obj";
    const std::string TERRAIN_MESH_PATH = "assets/models/Terrain.obj";
}

RenderingSystem::RenderingSystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &RenderingSystem::load, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &RenderingSystem::handle_key_press, this);
    EventSystem::add_event_handler(EventType::ADD_EXAMPLE_SHIP_EVENT, &RenderingSystem::handle_add_example_ship, this);
    EventSystem::add_event_handler(EventType::ADD_TERRAIN_EVENT, &RenderingSystem::handle_add_terrain, this);
    EventSystem::add_event_handler(EventType::EXAMPLE_SHIP_IDLE_EVENT, &RenderingSystem::handle_example_ship_idle, this);
    EventSystem::add_event_handler(EventType::OBJECT_TRANSFORM_EVENT, &RenderingSystem::handle_object_transform, this);
}

void RenderingSystem::update() {
}

void RenderingSystem::load(const Event& e) {
    init_window();
    setup_cameras();
}

void RenderingSystem::handle_key_press(const Event& e) {
    int player_id = e.get_value<int>("player_id", -1);
    int key = e.get_value<int>("key", -1);
    // int value = e.get_value<int>("value", 0);

    glm::mat4 transform;

    switch (key) {
        case SDLK_a:
            //transform = glm::rotate(glm::mat4(), 0.1f, glm::vec3(0, 1, 0));
            break;

        case SDLK_d:
            //transform = glm::rotate(glm::mat4(), -0.1f, glm::vec3(0, 1, 0));
            break;

        case SDLK_w:
            //transform = glm::rotate(glm::mat4(), 0.1f, glm::vec3(1, 0, 0));
            break;

        case SDLK_s:
            //transform = glm::rotate(glm::mat4(), -0.1f, glm::vec3(1, 0, 0));
            break;

        default:
            break;
    }

    for (auto& cam : cameras_) {
        cam *= transform;
    }

}

void RenderingSystem::handle_add_example_ship(const Event& e) {
    // Load game object parameters
    int object_id = e.get_value<int>("object_id", -1);
    assert(object_id != -1);

    int x = e.get_value<int>("pos_x", -999);
    assert(x != -999);

    int y = e.get_value<int>("pos_y", -999);
    assert(y != -999);

    int z = e.get_value<int>("pos_z", -999);
    assert(z != -999);

    // Load ship assets
    example_shader_.load_shader(
        SHIP_VERTEX_SHADER_PATH,
        SHIP_FRAGMENT_SHADER_PATH
    );

    MeshAsset* mesh = asset_manager_.get_mesh_asset(SHIP_MESH_PATH);

    // Store ship
    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(mesh);
    example_objects_[object_id].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(x, y, z)));
}

void RenderingSystem::handle_add_terrain(const Event& e) {
    // Load game object parameters
    int object_id = e.get_value<int>("object_id", -1);
    assert(object_id != -1);

    MeshAsset* mesh = asset_manager_.get_mesh_asset(TERRAIN_MESH_PATH);

    // Store terrain
    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(mesh);
}

void RenderingSystem::handle_example_ship_idle(const Event& e) {
    // Load game object parameters
    int object_id = e.get_value<int>("object_id", -1);
    assert(object_id != -1 && object_id < example_objects_.size());

    float rotation_rad = e.get_value<float>("rotation_rad", 0.0f);

    example_objects_[object_id].apply_transform(glm::rotate(glm::mat4x4(), rotation_rad, glm::vec3(1, 1, 1)));
}

void RenderingSystem::handle_object_transform(const Event& e) {
    int object_id = e.get_value<int>("object_id", -1);
    assert(object_id != -1);

    float x = e.get_value<float>("pos_x", -999);
    assert(x != -999);

    float y = e.get_value<float>("pos_y", -999);
    assert(y != -999);

    float z = e.get_value<float>("pos_z", -999);
    assert(z != -999);



    float qw = e.get_value<float>("qua_w", -999);
    assert(qw != -999);

    float qx = e.get_value<float>("qua_x", -999);
    assert(qx != -999);

    float qy = e.get_value<float>("qua_y", -999);
    assert(qy != -999);

    float qz = e.get_value<float>("qua_z", -999);
    assert(qz != -999);



    example_objects_[object_id].set_transform(glm::translate(glm::mat4(), glm::vec3(x, y, z)));
    example_objects_[object_id].apply_transform(glm::toMat4(glm::quat(qw, qx, qy, qz)));
}

void RenderingSystem::render() {
    start_render();
    setup_cameras();

    for (auto& object : example_objects_) {
        object.render_views(cameras_, 4, example_shader_.program_id_);
    }

    end_render();
}

bool RenderingSystem::init_window() {

    const int sdl_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    const int screen_width = 640;
    const int screen_height = 480;

    window_ = SDL_CreateWindow("WienerTakesAll",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               screen_width,
                               screen_height,
                               sdl_flags);

    if (window_ == NULL) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GLContext glContext = SDL_GL_CreateContext(window_);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK) {
        std::cout << glewGetErrorString(err) << std::endl;
        return false;
    }


    glGenVertexArrays(1, &vertex_array_id_);
    glBindVertexArray(vertex_array_id_);

    return true;
}

void RenderingSystem::start_render() const {
    //Clear the buffers and setup the opengl requirements
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glUseProgram(example_shader_.program_id_);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
}

void RenderingSystem::setup_cameras() {
    glm::mat4 P = glm::perspective(glm::radians(60.f), 4.0f / 3.0f, 0.1f, 100.0f);

    glm::mat4x4 transform;
    if (example_objects_.size())
         transform = example_objects_[0].get_transform();

    glm::vec3 car_pos(transform[3][0], transform[3][1] + 0.5, transform[3][2]);

    cameras_[0] = glm::translate(transform, glm::vec3(0,3,-5));
    cameras_[0] = P * glm::lookAt(glm::vec3(cameras_[0][3]), car_pos, glm::vec3(0, 1, 0));

    cameras_[1] = glm::translate(glm::mat4(), glm::vec3(5.f, 5.f, 0.f));
    cameras_[1] = P * glm::lookAt(glm::vec3(cameras_[1][3]), car_pos, glm::vec3(0, 1, 0));

    cameras_[2] = glm::translate(transform, glm::vec3(5, 3, 0));
    cameras_[2] = P * glm::lookAt(glm::vec3(cameras_[2][3]), car_pos, glm::vec3(0, 1, 0));

    cameras_[3] = glm::translate(glm::mat4(), glm::vec3(-1.5f, 2.f, -1.5f));
    cameras_[3] = P * glm::lookAt(glm::vec3(cameras_[3][3]), car_pos, glm::vec3(0, 1, 0));
}

void RenderingSystem::end_render() const {
    SDL_GL_SwapWindow(window_);
}
