#include <cassert>
#include <iostream>

#include "EventSystem.h"
#include "GameState.h"
#include "InputManager.h"
#include "InputSettings.h"
#include "SDL.h"

InputManager::InputManager(const InputSettings& settings)
    : settings_(settings)
    , controllers_(std::array<SDL_GameController *, 4>())
    , haptics_(std::array<SDL_Haptic *, 4>())
    , num_players_(4)
    , dom_(-1) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &InputManager::handle_load_event, this);
    EventSystem::add_event_handler(EventType::RELOAD_SETTINGS_EVENT, &InputManager::handle_reload_settings_event, this);
    EventSystem::add_event_handler(EventType::VEHICLE_COLLISION, &InputManager::handle_vehicle_collision, this);
    EventSystem::add_event_handler(EventType::NEW_GAME_STATE, &InputManager::handle_new_game_state, this);
    EventSystem::add_event_handler(EventType::DOMINATE_CONTROLS, &InputManager::handle_dominate_controls, this);
    EventSystem::add_event_handler(EventType::RESTORE_CONTROLS, &InputManager::handle_restore_controls, this);
    EventSystem::add_event_handler(EventType::REVERSE_CONTROLS, &InputManager::handle_reverse_controls, this);

    controllers_reversed_ = {false, false, false, false};
}

void InputManager::process_input(SDL_Event* event) {
    int key = -1;
    int value = -1;
    int player_id = -1;
    bool should_queue_event = true;

    switch (event->type) {
        // Keyboard
        case SDL_KEYUP: {
            value = SDL_KEYUP;
            key = event->key.keysym.sym;
            should_queue_event = process_keyboard(key, player_id);

            break;
        }

        case SDL_KEYDOWN: {
            value = SDL_KEYDOWN;
            key = event->key.keysym.sym;
            should_queue_event = process_keyboard(key, player_id);

            break;
        }

        case SDL_CONTROLLERBUTTONUP: {
            value = SDL_CONTROLLERBUTTONUP;
            player_id = get_player_id_from_joystick_index(event->cbutton.which); // Joystick ID of event sender
            key = event->cbutton.button;
            std::cout << "[Player " << player_id << "] ";
            should_queue_event = process_controller_button(key);
            break;
        }

        case SDL_CONTROLLERBUTTONDOWN: {
            value = SDL_CONTROLLERBUTTONDOWN;
            player_id = get_player_id_from_joystick_index(event->cbutton.which); // Joystick ID of event sender
            key = event->cbutton.button;
            std::cout << "[Player " << player_id << "] ";
            should_queue_event = process_controller_button(key);

            break;
        }

        case SDL_CONTROLLERAXISMOTION: {
            key = event->caxis.axis;
            value = event->caxis.value; // Current displacement of joystick
            player_id = get_player_id_from_joystick_index(event->caxis.which); // Joystick ID of event sender
            std::cout << "[Player " << player_id << "] ";
            should_queue_event = process_controller_axis(key, value);
            std::cout << " value: " << value << std::endl;

            break;
        }

        default:
            should_queue_event = false;
            break;

    }

    should_queue_event =    should_queue_event  &&
                            player_id != -1;

    if (!should_queue_event) {
        return;
    }

    if (controllers_reversed_[player_id]) {
        key = get_reverse_key(key);
    }

    if (dom_ == -1) {
        EventSystem::queue_event(
            Event(
                EventType::KEYPRESS_EVENT,
                "player_id", player_id,
                "key", key,
                "value", value
            )
        );
    }

    else if (player_id == dom_) {
        for (int i = 0; i < 4; ++i) {
            EventSystem::queue_event(
                Event(
                    EventType::KEYPRESS_EVENT,
                    "player_id", i,
                    "key", key,
                    "value", value
                )
            );
        }
    }

}

void InputManager::quit() {
    for (SDL_GameController* game_controller : controllers_) {
        if (game_controller != nullptr) {
            SDL_GameControllerClose(game_controller);
        }
    }

    for (SDL_Haptic* haptic : haptics_) {
        if (haptic == nullptr) {
            SDL_HapticClose(haptic);
        }
    }

    std::cout << "All controllers_ closed" << std::endl;
}

void InputManager::handle_reload_settings_event(const Event& event) {
    // Add any necessary updates here
}

void InputManager::handle_vehicle_collision(const Event& e) {
    int a_id = e.get_value<int>("a_id", true).first;
    rumble_controller(a_id);

    int b_id = e.get_value<int>("b_id", true).first;
    rumble_controller(b_id);
}

void InputManager::handle_new_game_state(const Event& e) {
    GameState state = static_cast<GameState>(e.get_value<int>("state", true).first);

    switch (state) {
        case GameState::START_MENU:
            dom_ = -1;
            break;

        case GameState::IN_GAME:
            num_players_ = e.get_value<int>("num_players", true).first;
            break;

        default:
            break;
    }
}

void InputManager::handle_dominate_controls(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    dom_ = object_id;
}

void InputManager::handle_restore_controls(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    // Case 1: Restore controls after domination
    if (object_id == dom_) {
        dom_ = -1;
    }

    // Case 2: Restore controls after reversal
    controllers_reversed_[object_id] = false;
}

void InputManager::handle_reverse_controls(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    controllers_reversed_[object_id] = true;
}

void InputManager::handle_load_event(const Event& e) {
    int num_controllers = SDL_NumJoysticks();

    // Link player controllers_
    SDL_GameControllerEventState(SDL_ENABLE);
    SDL_GameController* controller = nullptr;

    for (int controller_id = 0; controller_id < num_controllers; ++controller_id) {
        if (SDL_IsGameController(controller_id)) {
            // Link controller
            controller = SDL_GameControllerOpen(controller_id);

            if (!controller) {
                std::cerr << "Could not open GameController " << controller_id << ": " << SDL_GetError() << std::endl;
                continue;
            }

            int player_id = get_player_id_from_joystick_index(controller_id);
            controllers_[player_id] = controller;
            std::cout << "Controller for player " << player_id << " linked" << std::endl;

            // Open haptic
            SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
            SDL_Haptic* haptic = SDL_HapticOpenFromJoystick(joystick);
            player_id = controller_id;

            if (!haptic) {
                std::cerr << "Could not open haptic for player " << player_id << "'s controller" << std::endl;
                haptics_[player_id] = nullptr;
                continue;
            }

            // Initialize haptic rumble
            if (SDL_HapticRumbleInit(haptic) != 0) {
                std::cerr << "Could not initialize haptic rumble for player " << player_id << "'s controller" << std::endl;
                haptics_[player_id] = nullptr;
                continue;
            }

            haptics_[player_id] = haptic;
            std::cout << "Haptic rumble for player " << player_id << " also initialized" << std::endl;
        }
    }
}

bool InputManager::process_keyboard(const int& key, int& player_id) {
    bool is_key_mapped = true;

    /* Check the SDLKey values and move change the coords */
    switch ( key ) {
        case SDLK_q:
            player_id = 0;
            break;

        case SDLK_e:
            player_id = 0;
            break;

        case SDLK_w:
            player_id = 0;
            break;

        case SDLK_s:
            player_id = 0;
            break;

        case SDLK_a:
            player_id = 0;
            break;

        case SDLK_d:
            player_id = 0;
            break;

        case SDLK_r:
            player_id = 1;
            break;

        case SDLK_y:
            player_id = 1;
            break;

        case SDLK_t:
            player_id = 1;
            break;

        case SDLK_f:
            player_id = 1;
            break;

        case SDLK_g:
            player_id = 1;
            break;

        case SDLK_h:
            player_id = 1;
            break;

        case SDLK_u:
            player_id = 2;
            break;

        case SDLK_o:
            player_id = 2;
            break;

        case SDLK_i:
            player_id = 2;
            break;

        case SDLK_j:
            player_id = 2;
            break;

        case SDLK_k:
            player_id = 2;
            break;

        case SDLK_l:
            player_id = 2;
            break;

        case SDLK_RETURN:
            player_id = 3;
            break;

        case SDLK_RSHIFT:
            player_id = 3;
            break;

        case SDLK_LEFT:
            player_id = 3;
            break;

        case SDLK_UP:
            player_id = 3;
            break;

        case SDLK_DOWN:
            player_id = 3;
            break;

        case SDLK_RIGHT:
            player_id = 3;
            break;

        case SDLK_F5:
            player_id = 0;
            break;

        case SDLK_F11:
            player_id = 0;
            break;

        case SDLK_ESCAPE:
            player_id = 0;
            break;

        default:
            is_key_mapped = false;
            std::cout << "UNMAPPED" << std::endl;
            break;
    }

    return is_key_mapped;
}

bool InputManager::process_controller_button(const int& button) {
    bool is_mapped_button = true;

    switch (button) {

        // LETTER BUTTONS

        case SDL_CONTROLLER_BUTTON_A:
            break;

        case SDL_CONTROLLER_BUTTON_B:
            break;

        case SDL_CONTROLLER_BUTTON_X:
            break;

        case SDL_CONTROLLER_BUTTON_Y:
            break;

        // SHOULDER BUTTONS

        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            break;

        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            break;

        // SPECIAL BUTTONS

        case SDL_CONTROLLER_BUTTON_START:
            break;

        case SDL_CONTROLLER_BUTTON_BACK:
            break;

        case SDL_CONTROLLER_BUTTON_GUIDE:
            break;

        // DIRECTIONAL BUTTONS

        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            break;

        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            break;

        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            break;

        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            break;

        // STICK BUTTONS

        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            break;

        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            break;

        // MISC

        default:
            is_mapped_button = false;
            break;
    }

    return is_mapped_button;
}

bool InputManager::process_controller_axis(const int& axis, const int& value) {
    bool is_mapped_axis = true;

    if ((value > -settings_.deadzone) && (value < settings_.deadzone)) {
        //break;
    }


    switch (axis) {

        // LEFT STICK

        case SDL_CONTROLLER_AXIS_LEFTX:
            std::cout << "Left axis horizontal";
            break;

        case SDL_CONTROLLER_AXIS_LEFTY:
            std::cout << "Left axis vertical";
            break;

        // RIGHT STICK

        case SDL_CONTROLLER_AXIS_RIGHTX:
            std::cout << "Right axis horizontal";
            break;

        case SDL_CONTROLLER_AXIS_RIGHTY:
            std::cout << "Right axis vertical";
            break;

        // TRIGGERS

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            std::cout << "Left trigger";
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            std::cout << "Right trigger";
            break;

        // MISC

        default:
            is_mapped_axis = false;
            break;
    }

    return is_mapped_axis;
}

void InputManager::rumble_controller(const int id) const {
    if (    id >= haptics_.size()   ||
            id >= num_players_      ||
            haptics_[id] == nullptr ) {
        return;
    }

    SDL_HapticRumblePlay(haptics_[id], 0.5f, 500);
}

const int InputManager::get_player_id_from_joystick_index(const int joystick_index) const {
    int player_id = SDL_NumJoysticks() - joystick_index - 1;

    if (player_id >= num_players_) {
        return -1;
    }

    assert(player_id >= 0 && player_id < SDL_NumJoysticks());
    return player_id;
}

const int InputManager::get_reverse_key(const int key) const {
    int ret = -1;

    switch (key) {
        // self powerup
        case SDLK_q:
        case SDLK_r:
        case SDLK_u:
        case SDLK_RSHIFT:
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            ret = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
            break;

        // others powerup
        case SDLK_e:
        case SDLK_y:
        case SDLK_o:
        case SDLK_RETURN:
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            ret = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
            break;

        // Keyboard acceleration
        case SDLK_w:
        case SDLK_t:
        case SDLK_i:
        case SDLK_UP:
            ret = SDLK_DOWN;
            break;

        // keyboard braking
        case SDLK_s:
        case SDLK_g:
        case SDLK_k:
        case SDLK_DOWN:
            ret = SDLK_UP;
            break;

        // keyboard left steer
        case SDLK_a:
        case SDLK_f:
        case SDLK_j:
        case SDLK_LEFT:
            ret = SDLK_RIGHT;
            break;

        // keyboard right steer
        case SDLK_d:
        case SDLK_h:
        case SDLK_l:
        case SDLK_RIGHT:
            ret = SDLK_LEFT;
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            ret = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            ret = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
            break;

        case SDL_CONTROLLER_AXIS_LEFTX: {
            ret = SDL_CONTROLLER_AXIS_RIGHTX;
            break;
        }

        default:
            ret = -1;
    }

    return ret;
}
