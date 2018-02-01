#include "RenderingSystem.h"

#include "AssetManager.h"

#include <glm/gtc/matrix_transform.hpp>

RenderingSystem::RenderingSystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &RenderingSystem::load, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &RenderingSystem::handle_key_press, this);

	window_ = asset_manager.get_window();
}

void RenderingSystem::update() {
    example_objects_[1].apply_transform(glm::rotate(glm::mat4x4(), 0.01f, glm::vec3(1, 1, 1)));
}

void RenderingSystem::load(const Event& e) {
    init_window();

    MeshAsset* mesh = asset_manager_.get_mesh_asset("assets/models/teapot.obj");
	TextureAsset* texture = asset_manager_.get_texture_asset("assets/models/default.jpg");
	ShaderAsset* shader = asset_manager_.get_shader_asset("assets/shaders/SimpleShader");

    example_objects_.emplace_back();
    example_objects_[0].set_mesh(mesh);
	example_objects_[0].set_texture(texture);
	example_objects_[0].set_shader(shader);
    example_objects_[0].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(0, -2, 0)));
	example_objects_[0].apply_transform(glm::scale(glm::mat4x4(), glm::vec3(0.02f, 0.02f, 0.02f)));

    example_objects_.emplace_back();
    example_objects_[1].set_mesh(mesh);
	example_objects_[1].set_texture(texture);
	example_objects_[1].set_shader(shader);
    example_objects_[1].apply_transform(glm::translate(glm::mat4x4(), glm::vec3(1, 2, 1)));
	example_objects_[1].apply_transform(glm::scale(glm::mat4x4(), glm::vec3(0.02f, 0.02f, 0.02f)));

    setup_cameras();
}

void RenderingSystem::handle_key_press(const Event& e) {
    int player_id = e.get_value<int>("player_id", -1);
    int key = e.get_value<int>("key", -1);
    // int value = e.get_value<int>("value", 0);

    glm::mat4 transform;

    switch (key) {
        case SDLK_a:
            transform = glm::rotate(glm::mat4(), 0.1f, glm::vec3(0, 1, 0));
            break;

        case SDLK_d:
            transform = glm::rotate(glm::mat4(), -0.1f, glm::vec3(0, 1, 0));
            break;

        case SDLK_w:
            transform = glm::rotate(glm::mat4(), 0.1f, glm::vec3(1, 0, 0));
            break;

        case SDLK_s:
            transform = glm::rotate(glm::mat4(), -0.1f, glm::vec3(1, 0, 0));
            break;

        default:
            break;
    }

    for (auto& cam : cameras_) {
        cam *= transform;
    }

}

void RenderingSystem::render() {
    start_render();

    
	for (size_t i = 0; i < cameras_.size(); i++) {
		GLint x = 320 * (i % 2);
		GLint y = 240 - 240 * (i / 2);

		glViewport(x, y, 320, 240);
		for (auto& object : example_objects_) {
			object.render(cameras_[i]);
		}
	}
    

    end_render();
}

bool RenderingSystem::init_window() {
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
}

void RenderingSystem::setup_cameras() {
    glm::mat4 P = glm::perspective(glm::radians(60.f), 4.0f / 3.0f, 0.1f, 100.0f);

    cameras_[0] = glm::translate(glm::mat4(), glm::vec3(5.f * std::sin(0), 5.f * std::sin(0), 5.f * std::cos(0)));
    cameras_[0] = P * glm::lookAt(glm::vec3(cameras_[0][3]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    cameras_[1] = glm::translate(glm::mat4(), glm::vec3(5.f * std::cos(0), 5.f * std::cos(0), 5.f * std::sin(0)));
    cameras_[1] = P * glm::lookAt(glm::vec3(cameras_[1][3]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    cameras_[2] = glm::translate(glm::mat4(), glm::vec3(5.f * std::cos(std::sqrt(0)), 5.f * std::cos(0), 5.f * std::sin(0)));
    cameras_[2] = P * glm::lookAt(glm::vec3(cameras_[2][3]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    cameras_[3] = glm::translate(glm::mat4(), glm::vec3(15.f * std::sin(0), 15.f * std::sin(0), 15.f * std::cos(0)));
    cameras_[3] = P * glm::lookAt(glm::vec3(cameras_[3][3]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

void RenderingSystem::end_render() const {
    //SDL_GL_SwapWindow(window_);
}
