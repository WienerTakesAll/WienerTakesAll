#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "EventSystem.h"
#include "InputManager.h"
#include "InputSettings.h"
#include "SDL.h"

InputManager::InputManager(const InputSettings& settings)
    : settings_(settings) {
    EventSystem::add_event_handler(EventType::LOAD_EVENT, &InputManager::handle_load_event, this);
    EventSystem::add_event_handler(EventType::RELOAD_SETTINGS_EVENT, &InputManager::handle_reload_settings_event, this);
    EventSystem::add_event_handler(EventType::VEHICLE_COLLISION, &InputManager::handle_vehicle_collision, this);
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

    if (should_queue_event) {
        EventSystem::queue_event(
            Event(
                EventType::KEYPRESS_EVENT,
                "player_id", player_id,
                "key", key,
                "value", value
            )
        );
    }
}

void InputManager::quit() {
    for (SDL_GameController* game_controller : controllers_) {
        SDL_GameControllerClose(game_controller);
    }

    for (SDL_Haptic* haptic : haptics_) {
        SDL_HapticClose(haptic);
    }

    controllers_.clear();
    haptics_.clear();
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

void InputManager::handle_load_event(const Event& e) {
    int num_controllers_player = SDL_NumJoysticks();

    // Link player controllers_
    SDL_GameControllerEventState(SDL_ENABLE);
    SDL_GameController* controller = nullptr;

    for (int player_id = 0; player_id < num_controllers_player; ++player_id) {
        if (SDL_IsGameController(player_id)) {
            // Link controller
            controller = SDL_GameControllerOpen(player_id);

            if (!controller) {
                std::cerr << "Could not open GameController " << player_id << ": " << SDL_GetError() << std::endl;
                continue;
            }

            controllers_.push_back(controller);
            std::cout << "Controller for player " << player_id << " linked" << std::endl;

            // Open haptic
            SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
            SDL_Haptic* haptic = SDL_HapticOpenFromJoystick(joystick);

            if (!haptic) {
                std::cerr << "Could not open haptic for player " << player_id << "'s controller" << std::endl;
                haptics_.push_back(nullptr);
                continue;
            }

            // Initialize haptic rumble
            if (SDL_HapticRumbleInit(haptic) != 0) {
                std::cerr << "Could not initialize haptic rumble for player " << player_id << "'s controller" << std::endl;
                haptics_.push_back(nullptr);
                continue;
            }

            haptics_.push_back(haptic);
            std::cout << "Haptic rumble for player " << player_id << " also initialized" << std::endl;
        }
    }
}

bool InputManager::process_keyboard(const int& key, int& player_id) {
    bool is_key_mapped = true;

    /* Check the SDLKey values and move change the coords */
    switch ( key ) {
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

        case SDLK_SPACE:
            player_id = 0;
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
            break;

        case SDLK_RETURN:
            break;

        case SDLK_ESCAPE:
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
    if (id >= haptics_.size()) {
        return;
    }

    if (haptics_[id] == nullptr) {
        return;
    }

    SDL_HapticRumblePlay(haptics_[id], 0.5f, 500);
}

const int InputManager::get_player_id_from_joystick_index(const int joystick_index) const {
	int player_id = SDL_NumJoysticks() - joystick_index - 1;
	assert(player_id >= 0 && player_id < SDL_NumJoysticks());
	return player_id;
}
