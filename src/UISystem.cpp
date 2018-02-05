#include "UISystem.h"

#include "AssetManager.h"

namespace {
    const int UI_VIEW_PORT_WIDTH = 640;
    const int UI_VIEW_PORT_HEIGHT = 480;
}

UISystem::UISystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &UISystem::handle_load, this);

    // Hard-coded sample UI button mesh
    MeshAsset::VertexData vert;

    vert.position_ = { 0.0f, 0.0f, 1.0f };
    vert.uv_ = { 0.0f, 0.0f };
    example_mesh_.vertices_.emplace_back(vert);

    vert.position_ = { 1.0f, 0.0f, 1.0f };
    vert.uv_ = { 1.0f, 0.0f };
    example_mesh_.vertices_.emplace_back(vert);

    vert.position_ = { 1.0f, 1.0f, 1.0f };
    vert.uv_ = { 1.0f, 1.0f };
    example_mesh_.vertices_.emplace_back(vert);

    vert.position_ = { 0.0f, 1.0f, 1.0f };
    vert.uv_ = { 0.0f, 1.0f };
    example_mesh_.vertices_.emplace_back(vert);

    example_mesh_.indices_.push_back(0);
    example_mesh_.indices_.push_back(1);
    example_mesh_.indices_.push_back(2);
    example_mesh_.indices_.push_back(2);
    example_mesh_.indices_.push_back(3);
    example_mesh_.indices_.push_back(0);

    example_mesh_.valid_ = true;

    window_ = asset_manager.get_window();
}

void UISystem::update() {
}

void UISystem::render() const {
    start_render();

    for (auto ui_object : ui_objects_) {
        ui_object.render(glm::mat4());
    }

    SDL_GL_SwapWindow(window_);

    end_render();
}

void UISystem::start_render() const {
    glViewport(0, 0, UI_VIEW_PORT_WIDTH, UI_VIEW_PORT_HEIGHT);
    glDisable(GL_DEPTH_TEST);
}

void UISystem::end_render() const {

}

void UISystem::handle_load(const Event& e) {
    example_shader_ = asset_manager_.get_shader_asset("assets/shaders/SimpleShader");
    TextureAsset* tex = asset_manager_.get_texture_asset("assets/textures/default.png");

    ui_objects_.push_back(UIObject(glm::vec2(-0.25f), glm::vec3(1.0f), glm::vec2(0.5f), &example_mesh_, tex, example_shader_));
}

void UISystem::handle_key_press(const Event& e) {
    // Change active_button_
}
