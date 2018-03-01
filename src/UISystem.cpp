#include "UISystem.h"

#include "AssetManager.h"

namespace {
    const int UI_VIEW_PORT_WIDTH = 640;
    const int UI_VIEW_PORT_HEIGHT = 480;
}

UISystem::UISystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager)
    , start_menu_(asset_manager)
    , current_game_state_(GameState::START_MENU) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &UISystem::handle_load, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &UISystem::handle_key_press, this);

    window_ = asset_manager.get_window();
}

void UISystem::update() {
}

void UISystem::render() const {
    start_render();

    if(current_game_state_ == GameState::START_MENU) {
        start_menu_.render();
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
    start_menu_.load();
}

void UISystem::handle_key_press(const Event& e) {
    int key = e.get_value<int>("key", true).first;
    if(current_game_state_ == GameState::START_MENU) {
        if (key == SDLK_RETURN) {
            EventSystem::queue_event(
                Event(
                    EventType::NEW_GAME_STATE,
                    "state", GameState::IN_GAME
                )
            );
            current_game_state_ = GameState::IN_GAME;
        }
    }
}
