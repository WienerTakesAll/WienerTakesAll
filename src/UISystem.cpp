#include "UISystem.h"

UISystem::UISystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &UISystem::handle_load, this);
}

void UISystem::update() {
}

void UISystem::render() const {

    for (auto ui_object : ui_objects_) {
        ui_object.render();
    }

}

void UISystem::start_render() const {

}

void UISystem::end_render() const {

}

void UISystem::handle_load(const Event& e) {
    example_shader_.load_shader(
        "assets/shaders/SimpleVertexShader.vertexshader",
        "assets/shaders/SimpleFragmentShader.fragmentshader");

    ui_objects_.push_back(UIObject(glm::vec2(0.0f), glm::vec3(1.0f), 0.25f, 0.25f));
}

void UISystem::handle_key_press(const Event& e) {
    // Change active_button_
}
