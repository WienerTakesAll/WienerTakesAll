#include "RenderingSystem.h"

#include "AssetManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace {
    const std::string STANDARD_SHADER_PATH = "assets/shaders/SimpleShader";
    const std::string SHADOW_SHADER_PATH = "assets/shaders/ShadowShader";
    const std::string CAR_MESH_PATH = "assets/models/carBoxModel.obj";
    const std::string TERRAIN_MESH_PATH = "assets/models/Arena.obj";
}

RenderingSystem::RenderingSystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {
    window_ = asset_manager.get_window();


    EventSystem::add_event_handler(EventType::LOAD_EVENT, &RenderingSystem::load, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &RenderingSystem::handle_key_press, this);
    EventSystem::add_event_handler(EventType::ADD_VEHICLE, &RenderingSystem::handle_add_vehicle, this);
    EventSystem::add_event_handler(EventType::ADD_ARENA, &RenderingSystem::handle_add_terrain, this);
    EventSystem::add_event_handler(EventType::OBJECT_TRANSFORM_EVENT, &RenderingSystem::handle_object_transform, this);

    init_window();
}

void RenderingSystem::update() {
}

void RenderingSystem::load(const Event& e) {
    setup_cameras();

    shadow_shader_ = asset_manager_.get_shader_asset(SHADOW_SHADER_PATH);
}

void RenderingSystem::handle_key_press(const Event& e) {
    // function calls to get_value: param1= string:name, param2 = bool:crash_on_fail
    // pair.first == the int, pair.second == bool
    // std::pair<int, bool> player_id = e.get_value<int>("player_id", true);
    std::pair<int, bool> key = e.get_value<int>("key", true);
    // std::pair<int, bool> value = e.get_value<int>("value", true);

    glm::mat4 transform;

    switch (key.first) {
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

void RenderingSystem::handle_add_vehicle(const Event& e) {
    // Load game object parameters
    std::pair<int, bool> object_id = e.get_value<int>("object_id", true);
    assert(object_id.second);

    std::pair<int, bool> x = e.get_value<int>("pos_x", true);
    assert(x.second);

    std::pair<int, bool> y = e.get_value<int>("pos_y", true);
    assert(y.second);

    std::pair<int, bool> z = e.get_value<int>("pos_z", true);
    assert(z.second);


    MeshAsset* mesh = asset_manager_.get_mesh_asset(CAR_MESH_PATH);

    // Store car
    example_objects_.emplace_back();
    example_objects_[object_id.first].set_mesh(mesh);
    example_objects_[object_id.first].set_shader(asset_manager_.get_shader_asset(STANDARD_SHADER_PATH));
    example_objects_[object_id.first].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(x.first, y.first, z.first)));

    car_indices_.push_back(object_id.first);
}

void RenderingSystem::handle_add_terrain(const Event& e) {
    // Load game object parameters
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(TERRAIN_MESH_PATH);

    // Store terrain
    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(mesh);
    example_objects_[object_id].set_shader(asset_manager_.get_shader_asset(STANDARD_SHADER_PATH));
}

void RenderingSystem::handle_object_transform(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    float x = e.get_value<float>("pos_x", true).first;
    float y = e.get_value<float>("pos_y", true).first;
    float z = e.get_value<float>("pos_z", true).first;

    float qw = e.get_value<float>("qua_w", true).first;
    float qx = e.get_value<float>("qua_x", true).first;
    float qy = e.get_value<float>("qua_y", true).first;
    float qz = e.get_value<float>("qua_z", true).first;

    example_objects_[object_id].set_transform(glm::translate(glm::mat4(), glm::vec3(x, y, z)));
    example_objects_[object_id].apply_transform(glm::toMat4(glm::quat(qw, qx, qy, qz)));
}

void RenderingSystem::render() {
    start_render();
    setup_cameras();

    for (size_t i = 0; i < cameras_.size(); i++) {
        int vx = 320 * (i % 2);
        int vy = 240 * (i < 2);

        glViewport(vx, vy, 320, 240);


        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        for (auto& object : example_objects_) {
            object.render(cameras_[i], 0.3f);
        }

        for (auto& object : example_objects_) {
            object.render_lighting(cameras_[i], glm::vec3(-1.f, -1.f, 0.f), shadow_shader_);
        }


        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_EQUAL);
        glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
        glBlendEquation(GL_FUNC_ADD);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 0, ~0);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilMask(0);

        glDepthMask(GL_FALSE);

        for (auto& object : example_objects_) {
           object.render(cameras_[i], 0.f);
        }
    
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);

    }

    end_render();
}


bool RenderingSystem::init_window() {


    SDL_GLContext context = SDL_GL_CreateContext(window_);

    if (!context) {
        fprintf(stderr, "Couldn't create context: %s\n", SDL_GetError());
        return false;
    }

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK) {
        std::cout << "Error intitializing glew:" << glewGetErrorString(err) << std::endl;
        return false;
    }

    glGenVertexArrays(1, &vertex_array_id_);
    glBindVertexArray(vertex_array_id_);

    return true;
}

void RenderingSystem::start_render() const {
    //Clear the buffers and setup the opengl requirements
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glStencilMask(~0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderingSystem::setup_cameras() {
    glm::mat4 P = glm::perspective(glm::radians(60.f), 4.0f / 3.0f, 0.1f, 100.0f);

    glm::mat4x4 transform;

    for (size_t i = 0; i < car_indices_.size(); i++) {
        transform = example_objects_[car_indices_[i]].get_transform();
        glm::vec3 car_pos(transform[3][0], transform[3][1] + 0.5, transform[3][2]);

        cameras_[i] = glm::translate(transform, glm::vec3(0, 3, -5));
        cameras_[i] = P * glm::lookAt(glm::vec3(cameras_[i][3]), car_pos, glm::vec3(0, 1, 0));
    }
}

void RenderingSystem::end_render() const {
    //SDL_GL_SwapWindow(window_);
}
