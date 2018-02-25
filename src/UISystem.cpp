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
    MeshAsset::MeshData::VertexData vert;


    example_mesh_.meshes_.emplace_back();
    auto& mesh_element = example_mesh_.meshes_.back();

    vert.position_[0] = 0.0f;
    vert.position_[1] = 0.0f;
    vert.position_[2] = 1.0f;
    vert.uv_[0] = 0.0f;
    vert.uv_[1] = 0.0f;
    mesh_element.vertices_.emplace_back(vert);

    vert.position_[0] = 1.0f;
    vert.position_[1] = 0.0f;
    vert.position_[2] = 1.0f;
    vert.uv_[0] = 1.0f;
    vert.uv_[1] = 0.0f;
    mesh_element.vertices_.emplace_back(vert);

    vert.position_[0] = 1.0f;
    vert.position_[1] = 1.0f;
    vert.position_[2] = 1.0f;
    vert.uv_[0] = 1.0f;
    vert.uv_[1] = 1.0f;
    mesh_element.vertices_.emplace_back(vert);

    vert.position_[0] = 0.0f;
    vert.position_[1] = 1.0f;
    vert.position_[2] = 1.0f;
    vert.uv_[0] = 0.0f;
    vert.uv_[1] = 1.0f;
    mesh_element.vertices_.emplace_back(vert);

    mesh_element.indices_.push_back(0);
    mesh_element.indices_.push_back(1);
    mesh_element.indices_.push_back(2);
    mesh_element.indices_.push_back(2);
    mesh_element.indices_.push_back(3);
    mesh_element.indices_.push_back(0);

    example_mesh_.valid_ = true;

    window_ = asset_manager.get_window();
}

void UISystem::update() {
}

void UISystem::render() const {
    start_render();

    example_ui_object_.render(glm::mat4());

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
    example_shader_ = asset_manager_.get_shader_asset("assets/shaders/UIShader");
    TextureAsset* tex = asset_manager_.get_texture_asset("assets/textures/default.png");

    example_ui_object_ = UIObject(glm::vec2(-0.25f), glm::vec3(1.0f), glm::vec2(0.5f), &example_mesh_, tex, example_shader_);
}

void UISystem::handle_key_press(const Event& e) {
    // Change active_button_
}
