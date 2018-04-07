#include "RenderingSystem.h"

#include "AssetManager.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "GameState.h"
#include "Powerup.h"

namespace {
    const std::string STANDARD_SHADER_PATH = "assets/shaders/SimpleShader";
    const std::string TEXTURE_SHADER_PATH = "assets/shaders/TextureShader";
    const std::string SKYBOX_SHADER_PATH = "assets/shaders/SkyboxShader";
    const std::string SHADOW_SHADER_PATH = "assets/shaders/ShadowShader";
    const std::string CAR_MESH_PATH = "assets/models/NoWienerCarModel.obj";
    const std::string CAR_SHADOW_MESH_PATH = "assets/models/CarShadowModel.obj";
    const std::string WEINER_MESH_PATH = "assets/models/WienerCarModel.obj";
    const std::string CAR_TEXTURE_PATH = "assets/textures/textureCar.png";
    const std::string TERRAIN_MESH_PATH = "assets/models/TableMap.obj";
    const std::string TERRAIN_TEXTURE_PATH = "assets/textures/TableText.png";

    const std::string BASKET_MESH_PATH = "assets/models/basket.obj";
    const std::string BASKET_TEXTURE_PATH = "assets/textures/basketText.png";

    const std::string FRUIT_MESH_PATH = "assets/models/fruit.obj";
    const std::string FRUIT_TEXTURE_PATH = "assets/textures/fruitText.png";

    const std::string CHEESE_MESH_PATH = "assets/models/cheese.obj";
    const std::string CHEESE_TEXTURE_PATH = "assets/textures/cheeseText.png";

    const std::string SKYBOX_MESH_PATH = "assets/models/Skybox.obj";
    const std::string SKYBOX_TEXTURE_PATH = "assets/textures/park.png";
    const std::string PACKET_MESH_PATH = "assets/models/Packet.obj";
    const std::string KETCHUP_TEXTURE_PATH = "assets/textures/KetchupPacket.png";
    const std::string RELISH_TEXTURE_PATH = "assets/textures/RelishPacket.png";
    const std::string MUSTARD_TEXTURE_PATH = "assets/textures/MustardPacket.png";
    const std::string CHARCOAL_MESH_PATH = "assets/models/Mound.obj";
    const std::string CHARCOAL_TEXTURE_PATH = "assets/textures/smouldering-charcoal.png";

    const int CAMERA_LAG_FRAMES = 5;
}

RenderingSystem::RenderingSystem(AssetManager& asset_manager)
    : car_speeds_( {
    0.f, 0.f, 0.f, 0.f
})
, asset_manager_(asset_manager)
, particle_subsystem_(asset_manager)
, whos_it(0) {
    window_ = asset_manager.get_window();

    EventSystem::add_event_handler(EventType::LOAD_EVENT, &RenderingSystem::load, this);
    EventSystem::add_event_handler(EventType::ADD_VEHICLE, &RenderingSystem::handle_add_vehicle, this);
    EventSystem::add_event_handler(EventType::ADD_ARENA, &RenderingSystem::handle_add_terrain, this);
    EventSystem::add_event_handler(EventType::ADD_CHARCOAL, &RenderingSystem::handle_add_charcoal, this);

    EventSystem::add_event_handler(EventType::ADD_BASKET, &RenderingSystem::handle_add_basket, this);
    EventSystem::add_event_handler(EventType::ADD_FRUIT, &RenderingSystem::handle_add_fruit, this);
    EventSystem::add_event_handler(EventType::ADD_CHEESE, &RenderingSystem::handle_add_cheese, this);

    EventSystem::add_event_handler(EventType::OBJECT_TRANSFORM_EVENT, &RenderingSystem::handle_object_transform, this);
    EventSystem::add_event_handler(EventType::NEW_IT, &RenderingSystem::handle_new_it, this);
    EventSystem::add_event_handler(EventType::NEW_GAME_STATE, &RenderingSystem::handle_new_game_state, this);
    EventSystem::add_event_handler(EventType::ADD_SKYBOX, &RenderingSystem::handle_add_skybox, this);
    EventSystem::add_event_handler(EventType::ADD_POWERUP, &RenderingSystem::handle_add_powerup, this);
    EventSystem::add_event_handler(EventType::CHANGE_POWERUP, &RenderingSystem::handle_change_powerup, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &RenderingSystem::handle_keypress, this);
    EventSystem::add_event_handler(EventType::USE_POWERUP, &RenderingSystem::handle_use_powerup, this);
    EventSystem::add_event_handler(EventType::FINISH_POWERUP, &RenderingSystem::handle_finish_powerup, this);

    init_window();
}

void RenderingSystem::update() {
    particle_subsystem_.update();
}

void RenderingSystem::load(const Event& e) {
    setup_cameras();
    preload_assets();
    shadow_shader_ = asset_manager_.get_shader_asset(SHADOW_SHADER_PATH);
    particle_subsystem_.handle_load(e);
    asset_manager_.toggle_fullscreen();
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
    MeshAsset* shadow_mesh = asset_manager_.get_mesh_asset(CAR_SHADOW_MESH_PATH);

    // Store car
    example_objects_.emplace_back();
    example_objects_[object_id.first].set_mesh(mesh);
    example_objects_[object_id.first].set_shadow_mesh(shadow_mesh);

    example_objects_[object_id.first].set_shader(asset_manager_.get_shader_asset(TEXTURE_SHADER_PATH));
    example_objects_[object_id.first].set_texture(asset_manager_.get_texture_asset(CAR_TEXTURE_PATH));
    example_objects_[object_id.first].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(x.first, y.first, z.first)));
    example_objects_[object_id.first].set_has_shadows(true);
    car_indices_.push_back(object_id.first);

}

void RenderingSystem::handle_add_terrain(const Event& e) {
    // Load game object parameters
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(TERRAIN_MESH_PATH);
    ShaderAsset* shader = asset_manager_.get_shader_asset(TEXTURE_SHADER_PATH);
    TextureAsset* texture = asset_manager_.get_texture_asset(TERRAIN_TEXTURE_PATH);

    // Store charcoal
    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(mesh);
    example_objects_[object_id].set_shader(shader);
    example_objects_[object_id].set_texture(texture);
}

void RenderingSystem::handle_add_basket(const Event& e) {
    // Load game object parameters
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(BASKET_MESH_PATH);
    ShaderAsset* shader = asset_manager_.get_shader_asset(TEXTURE_SHADER_PATH);
    TextureAsset* texture = asset_manager_.get_texture_asset(BASKET_TEXTURE_PATH);

    // Store charcoal
    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(mesh);
    example_objects_[object_id].set_shader(shader);
    example_objects_[object_id].set_texture(texture);
    //example_objects_[object_id].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(40.0, 1.0, 10.0)));
    example_objects_[object_id].set_has_shadows(false);
}
void RenderingSystem::handle_add_fruit(const Event& e) {
    // Load game object parameters
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(FRUIT_MESH_PATH);
    ShaderAsset* shader = asset_manager_.get_shader_asset(TEXTURE_SHADER_PATH);
    TextureAsset* texture = asset_manager_.get_texture_asset(FRUIT_TEXTURE_PATH);

    // Store terrain
    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(mesh);
    example_objects_[object_id].set_shader(shader);
    example_objects_[object_id].set_texture(texture);
    //example_objects_[object_id].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(x.first, 0.0, z.first)));
    example_objects_[object_id].set_has_shadows(false);
}
void RenderingSystem::handle_add_cheese(const Event& e) {
    // Load game object parameters
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(CHEESE_MESH_PATH);
    ShaderAsset* shader = asset_manager_.get_shader_asset(TEXTURE_SHADER_PATH);
    TextureAsset* texture = asset_manager_.get_texture_asset(CHEESE_TEXTURE_PATH);

    // Store terrain
    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(mesh);
    example_objects_[object_id].set_shader(shader);
    example_objects_[object_id].set_texture(texture);
    example_objects_[object_id].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(10.0, 0.0, 20.0)));
    example_objects_[object_id].set_has_shadows(false);
}

void RenderingSystem::handle_add_charcoal(const Event& e) {
    // Load game object parameters
    int object_id = e.get_value<int>("object_id", true).first;

    std::pair<int, bool> x = e.get_value<int>("pos_x", true);
    std::pair<int, bool> y = e.get_value<int>("pos_y", true);
    std::pair<int, bool> z = e.get_value<int>("pos_z", true);

    MeshAsset* mesh = asset_manager_.get_mesh_asset(CHARCOAL_MESH_PATH);
    ShaderAsset* shader = asset_manager_.get_shader_asset(TEXTURE_SHADER_PATH);
    TextureAsset* texture = asset_manager_.get_texture_asset(CHARCOAL_TEXTURE_PATH);

    // Store terrain
    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(mesh);
    example_objects_[object_id].set_shader(shader);
    example_objects_[object_id].set_texture(texture);
    example_objects_[object_id].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(x.first, y.first, z.first)));
    example_objects_[object_id].set_has_shadows(false);
}

void RenderingSystem::handle_object_transform(const Event& e) {
    particle_subsystem_.handle_object_transform(e);

    int object_id = e.get_value<int>("object_id", true).first;

    assert(object_id >= 0);

    if (example_objects_.size() <= static_cast<size_t>(object_id)) {
        return;
    }

    float x = e.get_value<float>("pos_x", true).first;
    float y = e.get_value<float>("pos_y", true).first;
    float z = e.get_value<float>("pos_z", true).first;

    float qw = e.get_value<float>("qua_w", true).first;
    float qx = e.get_value<float>("qua_x", true).first;
    float qy = e.get_value<float>("qua_y", true).first;
    float qz = e.get_value<float>("qua_z", true).first;

    example_objects_[object_id].set_transform(glm::translate(glm::mat4(), glm::vec3(x, y, z)));
    example_objects_[object_id].apply_transform(glm::toMat4(glm::quat(qw, qx, qy, qz)));

    auto e_vx = e.get_value<float>("vel_x", false);

    if (e_vx.second && object_id < 4) {
        float vx = e_vx.first;
        // float vy = e.get_value<float>("vel_y", true).first;
        float vz = e.get_value<float>("vel_z", true).first;

        car_speeds_[object_id] = glm::length(glm::vec2(vx, vz));
    }

}


void RenderingSystem::handle_new_it(const Event& e) {
    particle_subsystem_.handle_new_it(e);

    MeshAsset* bun_mesh = asset_manager_.get_mesh_asset(CAR_MESH_PATH);
    MeshAsset* dog_mesh = asset_manager_.get_mesh_asset(WEINER_MESH_PATH);

    example_objects_[whos_it].set_mesh(bun_mesh);

    whos_it = e.get_value<int>("object_id", true).first;

    example_objects_[whos_it].set_mesh(dog_mesh);
}


void RenderingSystem::handle_new_game_state(const Event& e) {
    GameState new_game_state = (GameState)e.get_value<int>("state", true).first;
    particle_subsystem_.handle_new_game_state(e);

    if (new_game_state == GameState::START_MENU) {
        example_objects_.clear();
        car_indices_.clear();
    }
}

void RenderingSystem::handle_add_skybox(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    MeshAsset* skybox_mesh = asset_manager_.get_mesh_asset(SKYBOX_MESH_PATH);
    ShaderAsset* skybox_shader = asset_manager_.get_shader_asset(SKYBOX_SHADER_PATH);
    TextureAsset* skybox_texture = asset_manager_.get_texture_asset(SKYBOX_TEXTURE_PATH);

    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(skybox_mesh);
    example_objects_[object_id].set_shader(skybox_shader);
    example_objects_[object_id].set_texture(skybox_texture);
}

void RenderingSystem::handle_add_powerup(const Event& e) {
    PowerupType powerup_type = static_cast<PowerupType>(e.get_value<int>("type", true).first);

    assert(powerup_type != PowerupType::POWERUP_COUNT);

    int object_id = e.get_value<int>("object_id", true).first;
    float x = e.get_value<float>("pos_x", true).first;
    float y = e.get_value<float>("pos_y", true).first;
    float z = e.get_value<float>("pos_z", true).first;

    MeshAsset* mesh = asset_manager_.get_mesh_asset(PACKET_MESH_PATH);;
    TextureAsset* texture;

    switch (powerup_type) {
        case PowerupType::KETCHUP:
            texture = asset_manager_.get_texture_asset(KETCHUP_TEXTURE_PATH);
            break;

        case PowerupType::RELISH:
            texture = asset_manager_.get_texture_asset(RELISH_TEXTURE_PATH);
            break;

        case PowerupType::MUSTARD:
            texture = asset_manager_.get_texture_asset(MUSTARD_TEXTURE_PATH);
            break;

        default:
            assert(false);
            break;
    }

    example_objects_.emplace_back();
    example_objects_[object_id].set_mesh(mesh);
    example_objects_[object_id].set_shader(asset_manager_.get_shader_asset(TEXTURE_SHADER_PATH));
    example_objects_[object_id].set_texture(texture);
    example_objects_[object_id].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(x, y, z)));
    example_objects_[object_id].set_has_shadows(true);
}

void RenderingSystem::handle_change_powerup(const Event& e) {
    PowerupType powerup_type = static_cast<PowerupType>(e.get_value<int>("type", true).first);

    assert(powerup_type != PowerupType::POWERUP_COUNT);

    int object_id = e.get_value<int>("object_id", true).first;
    TextureAsset* texture;

    switch (powerup_type) {
        case PowerupType::KETCHUP:
            texture = asset_manager_.get_texture_asset(KETCHUP_TEXTURE_PATH);
            break;

        case PowerupType::RELISH:
            texture = asset_manager_.get_texture_asset(RELISH_TEXTURE_PATH);
            break;

        case PowerupType::MUSTARD:
            texture = asset_manager_.get_texture_asset(MUSTARD_TEXTURE_PATH);
            break;

        default:
            assert(false);
            break;
    }

    example_objects_[object_id].set_texture(texture);
}

void RenderingSystem::handle_keypress(const Event& e) {
    int key = e.get_value<int>("key", true).first;
    int value = e.get_value<int>("value", true).first;

    if (key == SDLK_F11 && value == SDL_KEYDOWN) {
        asset_manager_.toggle_fullscreen();
    }

    if (key == SDLK_F5 && value == SDL_KEYDOWN) {
        assert(false);
    }
}

void RenderingSystem::handle_use_powerup(const Event& e) {
    particle_subsystem_.handle_use_powerup(e);

    PowerupType type = static_cast<PowerupType>(e.get_value<int>("type", true).first);
    PowerupTarget target = static_cast<PowerupTarget>(e.get_value<int>("target", true).first);
    int player_id = e.get_value<int>("index", true).first;
    const float overlay_intensity = 0.15f;
    glm::vec4 overlay = glm::vec4(0.0f);

    switch (type) {
        case PowerupType::KETCHUP:
            overlay[0] = overlay_intensity; // red
            break;

        case PowerupType::RELISH:
            overlay[1] = overlay_intensity; // green
            break;

        case PowerupType::MUSTARD:
            // red + green = yellow
            overlay[0] = overlay_intensity;
            overlay[1] = overlay_intensity;
            break;

        default:
            assert(false);
            break;
    }

    switch (target) {
        case PowerupTarget::SELF:
            example_objects_[player_id].set_colour_overlay(overlay);
            break;

        case PowerupTarget::OTHERS:
            for (int i = 0; i < 4; ++i) {
                if (i != player_id) {
                    example_objects_[i].set_colour_overlay(overlay);
                }
            }

            break;

        default:
            assert(false);
            break;
    }
}

void RenderingSystem::handle_finish_powerup(const Event& e) {
    particle_subsystem_.handle_finish_powerup(e);

    int object_id = e.get_value<int>("object_id", true).first;
    example_objects_[object_id].set_colour_overlay(glm::vec4());
}

void RenderingSystem::render() {
    start_render();
    setup_cameras();

    auto cameras = cameras_queue_.front();

    for (size_t i = 0; i < cameras.size(); i++) {

        int window_w, window_h;
        SDL_GetWindowSize(asset_manager_.get_window(), &window_w, &window_h);


        int vx = (window_w / 2) * (i % 2);
        int vy = (window_h / 2) * (i < 2);

        glViewport(vx, vy, (window_w / 2), (window_h / 2));


        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        for (auto& object : example_objects_) {
            object.render(cameras[i], 0.3f);
        }

        particle_subsystem_.render(cameras[i], i);

        for (auto& object : example_objects_) {
            object.render_lighting(cameras[i], glm::vec3(-0.4f, -1.0f, 0.f), shadow_shader_);
        }

        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_EQUAL);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 0, ~0);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilMask(0);

        glDepthMask(GL_FALSE);

        for (auto& object : example_objects_) {
            object.render(cameras[i], 0.f);
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderingSystem::setup_cameras() {
    std::array<glm::mat4x4, 4> new_cameras;
    glm::mat4x4 transform;

    // get camera setup for all 4 car's current positioning
    for (size_t i = 0; i < car_indices_.size(); i++) {
        transform = example_objects_[car_indices_[i]].get_transform();

        glm::vec3 car_pos(transform[3][0], transform[3][1] + 0.5, transform[3][2]);

        auto camera_position = glm::translate(transform, glm::vec3(0, 2.5, -7));

        {
            glm::vec3 bad_camera_pos(camera_position[3]);
            glm::vec3 old_camera_pos(last_cameras_pos_[i]);
            glm::vec3 smooth_camera_pos
                = bad_camera_pos * 0.25f + old_camera_pos * 0.75f;

            camera_position[3] = glm::vec4(smooth_camera_pos, 1.0);
        }


        if (camera_position[3].y < 4) {
            camera_position[3].y = 4;
        }

        glm::vec3 lookAtPos(car_pos.x, camera_position[3][1], car_pos.z);


        float FOV = 60.f + car_speeds_[i] * 5.f;
        glm::mat4 P = glm::perspective(glm::radians(FOV), 4.0f / 3.0f, 0.1f, 1000.0f);

        new_cameras[i] = P * glm::lookAt(glm::vec3(camera_position[3]), lookAtPos, glm::vec3(0, 1, 0));

        last_cameras_pos_[i] = glm::vec3(camera_position[3]);
    }

    // push camera setup to back of queue
    cameras_queue_.push(new_cameras);

    if (cameras_queue_.size() > CAMERA_LAG_FRAMES) {
        // ensure the camera setup is the camera setup of
        // at most CAMERA_LAG_FRAMES ago
        // i.e. in a normal case we will be rendering the cameras based on where the cars were 5 frames ago
        cameras_queue_.pop();
    }
}

void RenderingSystem::end_render() const {
    //SDL_GL_SwapWindow(window_);
}

void RenderingSystem::preload_assets() const {
    // Shaders
    asset_manager_.get_shader_asset(STANDARD_SHADER_PATH);
    asset_manager_.get_shader_asset(TEXTURE_SHADER_PATH);
    asset_manager_.get_shader_asset(SKYBOX_SHADER_PATH);
    asset_manager_.get_shader_asset(SHADOW_SHADER_PATH);

    // Wiener
    asset_manager_.get_mesh_asset(CAR_MESH_PATH);
    asset_manager_.get_mesh_asset(CAR_SHADOW_MESH_PATH);
    asset_manager_.get_mesh_asset(WEINER_MESH_PATH);
    asset_manager_.get_texture_asset(CAR_TEXTURE_PATH);

    // Terrain
    asset_manager_.get_mesh_asset(TERRAIN_MESH_PATH);
    asset_manager_.get_texture_asset(TERRAIN_TEXTURE_PATH);

    // Skybox
    asset_manager_.get_mesh_asset(SKYBOX_MESH_PATH);
    asset_manager_.get_texture_asset(SKYBOX_TEXTURE_PATH);

    // Powerups
    asset_manager_.get_mesh_asset(PACKET_MESH_PATH);
    asset_manager_.get_texture_asset(KETCHUP_TEXTURE_PATH);
    asset_manager_.get_texture_asset(RELISH_TEXTURE_PATH);
    asset_manager_.get_texture_asset(MUSTARD_TEXTURE_PATH);

    // Obstacles
    asset_manager_.get_mesh_asset(CHARCOAL_MESH_PATH);
    asset_manager_.get_texture_asset(CHARCOAL_TEXTURE_PATH);
}
