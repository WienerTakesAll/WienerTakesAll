#include "UISystem.h"

#include "AssetManager.h"
#include "Powerup.h"

UISystem::UISystem(AssetManager& asset_manager)
    : asset_manager_(asset_manager)
    , loading_frames_counter_(0)
    , start_menu_(asset_manager)
    , loading_screen_(asset_manager)
    , gameplay_hud_(asset_manager)
    , end_game_screen_(asset_manager)
    , current_game_state_(GameState::START_MENU)
    , controller_buffer_(false) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &UISystem::handle_load, this);
    EventSystem::add_event_handler(EventType::KEYPRESS_EVENT, &UISystem::handle_key_press, this);
    EventSystem::add_event_handler(EventType::NEW_GAME_STATE, &UISystem::handle_new_game_state, this);
    EventSystem::add_event_handler(EventType::UPDATE_SCORE, &UISystem::handle_update_score, this);
    EventSystem::add_event_handler(EventType::UPDATE_DIRECTION_TO_IT, &UISystem::handle_update_direction_to_it, this);
    EventSystem::add_event_handler(EventType::NEW_IT, &UISystem::handle_new_it, this);
    EventSystem::add_event_handler(EventType::PICKUP_POWERUP, &UISystem::handle_pickup_powerup, this);
    EventSystem::add_event_handler(EventType::USE_POWERUP, &UISystem::handle_use_powerup, this);
    EventSystem::add_event_handler(EventType::ACTIVATE_AI, &UISystem::handle_activate_ai, this);

    window_ = asset_manager.get_window();
}

void UISystem::update() {
    if (loading_frames_counter_ < 4 && current_game_state_ == IN_GAME) {
        loading_frames_counter_++;
    }
}

void UISystem::render() const {
    start_render();

    switch (current_game_state_) {
        case GameState::START_MENU:
            start_menu_.render();
            break;

        case GameState::IN_GAME:
            if (loading_frames_counter_ < 3) {
                // render loading screen for first 3 frames of in game
                loading_screen_.render();
            } else {
                gameplay_hud_.render();
            }

            break;

        case GameState::END_GAME:
            gameplay_hud_.render();
            end_game_screen_.render();
            break;
    }

    SDL_GL_SwapWindow(window_);

    end_render();
}

void UISystem::start_render() const {
    int window_w, window_h;
    SDL_GetWindowSize(asset_manager_.get_window(), &window_w, &window_h);
    glViewport(0, 0, window_w, window_h);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void UISystem::end_render() const {

}

void UISystem::handle_load(const Event& e) {
    start_menu_.load();
    loading_screen_.load();
    gameplay_hud_.load();
    end_game_screen_.load();
}

void UISystem::handle_key_press(const Event& e) {
    int key = e.get_value<int>("key", true).first;
    int value = e.get_value<int>("value", true).first;

    if (value != SDL_KEYDOWN && value != SDL_CONTROLLERBUTTONDOWN && key != SDL_CONTROLLER_AXIS_LEFTY) {
        return;
    }

    if (current_game_state_ == GameState::START_MENU) {
        switch (key) {
            case SDL_CONTROLLER_BUTTON_A:
            case SDL_CONTROLLER_BUTTON_START:
            case SDLK_RETURN:
                if (start_menu_.selected_exit()) {
                    SDL_Event quit_event;
                    quit_event.type = SDL_QUIT;
                    SDL_PushEvent(&quit_event);
                } else {
                    EventSystem::queue_event(
                        Event(
                            EventType::NEW_GAME_STATE,
                            "state", GameState::IN_GAME,
                            "num_players", start_menu_.selected_num_of_players()
                        )
                    );
                    loading_frames_counter_ = 0;
                }

                break;

            case SDLK_UP:
                start_menu_.move_selection_up();
                break;

            case SDLK_DOWN:
                start_menu_.move_selection_down();
                break;

            case SDL_CONTROLLER_AXIS_LEFTY:

                if (std::abs(value) < 10000) {
                    controller_buffer_ = false;
                    break;
                }

                if (controller_buffer_) {
                    break;
                }


                controller_buffer_ = true;

                if (value > 0) {
                    start_menu_.move_selection_down();
                } else {
                    start_menu_.move_selection_up();
                }

                break;

        }
    }

    if (current_game_state_ == GameState::END_GAME) {
        switch (key) {
            case SDL_CONTROLLER_BUTTON_A:
            case SDL_CONTROLLER_BUTTON_START:
            case SDLK_RETURN:
                EventSystem::queue_event(
                    Event(
                        EventType::NEW_GAME_STATE,
                        "state", GameState::START_MENU
                    )
                );
                break;

            default:
                break;
        }
    }
}


void UISystem::handle_new_game_state(const Event& e) {
    GameState new_game_state = (GameState)e.get_value<int>("state", true).first;

    switch (new_game_state) {
        case GameState::START_MENU:
            gameplay_hud_.reset_scores();
            gameplay_hud_.reset_powerups();
            break;

        case GameState::IN_GAME:
            gameplay_hud_.set_num_humans(e.get_value<int>("num_players", true).first);
            loading_screen_.render();
            break;

        case GameState::END_GAME:
            if (e.get_value<int>("winner", false).second) {
                int winner_id = e.get_value<int>("winner", false).first;

                if (num_ai_ == 3 && winner_id != 0) {
                    //A special case was added where -2 makes the crown invisible.
                    end_game_screen_.set_winner(-2);
                } else {
                    end_game_screen_.set_winner(winner_id);
                }

            } else {
                end_game_screen_.set_winner(-1);
            }

            break;
    }

    current_game_state_ = new_game_state;
}


void UISystem::handle_update_score(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    int score = e.get_value<int>("score", true).first;
    int lock_frames = e.get_value<int>("lock_frames", true).first;

    gameplay_hud_.update_score(object_id, score, lock_frames);
}

void UISystem::handle_update_direction_to_it(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    float x = e.get_value<float>("x", true).first;
    float y = e.get_value<float>("y", true).first;
    float z = e.get_value<float>("z", true).first;

    gameplay_hud_.update_it_pointer(object_id, {x, y, z});
}

void UISystem::handle_new_it(const Event& e) {
    int new_it_id = e.get_value<int>("object_id", true).first;
    gameplay_hud_.new_it(new_it_id);
}

void UISystem::handle_pickup_powerup(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    int powerup_type = e.get_value<int>("powerup_type", true).first;

    gameplay_hud_.pickup_powerup(object_id, powerup_type);
}

void UISystem::handle_use_powerup(const Event& e) {
    int object_id = e.get_value<int>("index", true).first;
    gameplay_hud_.use_powerup(object_id);
}

void UISystem::handle_activate_ai(const Event& e) {
    num_ai_ = e.get_value<int>("num_ai", true).first;
    gameplay_hud_.set_num_ai(num_ai_);
}
