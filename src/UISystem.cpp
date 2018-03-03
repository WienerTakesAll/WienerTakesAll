#include "UISystem.h"

#include "AssetManager.h"

namespace {
    const int UI_VIEW_PORT_WIDTH = 640;
    const int UI_VIEW_PORT_HEIGHT = 480;
}

UISystem::UISystem(AssetManager& asset_manager)
    : start_menu_(asset_manager)
    , current_game_state_(GameState::START_MENU) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &UISystem::handle_load, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &UISystem::handle_key_press, this);

    window_ = asset_manager.get_window();
}

void UISystem::update() {
}

void UISystem::render() const {
    start_render();

    if (current_game_state_ == GameState::START_MENU) {
        start_menu_.render();
    }

    SDL_GL_SwapWindow(window_);

    end_render();
}

void UISystem::start_render() const {
    glViewport(0, 0, UI_VIEW_PORT_WIDTH, UI_VIEW_PORT_HEIGHT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
}

void UISystem::end_render() const {

}

void UISystem::handle_load(const Event& e) {
    start_menu_.load();
}

void UISystem::handle_key_press(const Event& e) {
    int key = e.get_value<int>("key", true).first;
    int value = e.get_value<int>("value", true).first;

    if (value != SDL_KEYDOWN) {
        return;
    }

    if (current_game_state_ == GameState::START_MENU) {
        switch (key) {
            case SDLK_RETURN:
                EventSystem::queue_event(
                    Event(
                        EventType::NEW_GAME_STATE,
                        "state", GameState::IN_GAME,
                        "num_players", start_menu_.selected_num_of_players()
                    )
                );
                current_game_state_ = GameState::IN_GAME;
                break;

            case SDLK_UP:
                start_menu_.move_selection_up();
                break;

            case SDLK_DOWN:
                start_menu_.move_selection_down();
                break;
        }
    }
}
