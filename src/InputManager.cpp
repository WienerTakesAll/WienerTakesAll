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
            player_id = event->cbutton.which; // Joystick ID of event sender
            key = event->cbutton.button;
            std::cout << "[Player " << player_id << "] ";
            should_queue_event = process_controller_button(key);

            break;
        }

        case SDL_CONTROLLERBUTTONDOWN: {
            value = SDL_CONTROLLERBUTTONDOWN;
            player_id = event->cbutton.which; // Joystick ID of event sender
            key = event->cbutton.button;
            std::cout << "[Player " << player_id << "] ";
            should_queue_event = process_controller_button(key);

            break;
        }

        case SDL_CONTROLLERAXISMOTION: {
            key = event->caxis.axis;
            value = event->caxis.value; // Current displacement of joystick
            player_id = event->caxis.which; // Joystick ID of event sender
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

    controllers_.clear();
    std::cout << "All controllers_ closed" << std::endl;
}

void InputManager::handle_reload_settings_event(const Event& event) {
    // Add any necessary updates here
}

void InputManager::handle_load_event(const Event& e) {
    int num_controllers_player = SDL_NumJoysticks();

    // Link player controllers_
    SDL_GameControllerEventState(SDL_ENABLE);
    SDL_GameController* controller = nullptr;

    for (int player_id = 0; player_id < num_controllers_player; ++player_id) {
        if (SDL_IsGameController(player_id)) {
            controller = SDL_GameControllerOpen(player_id);

            if (controller) {
                controllers_.push_back(controller);
                std::cout << "Controller for player " << player_id << " linked" << std::endl;
            } else {
                std::cerr << "Could not open GameController " << player_id << ": " << SDL_GetError() << std::endl;
            }

        }
    }

    // Create AI controllers_
    if (num_controllers_player < settings_.max_players) {
        for (int player_id = num_controllers_player; player_id < settings_.max_players; ++player_id) {
            // Create AI controller
            std::cout << "Create AI Controller for player " << player_id << " here" << std::endl;
        }
    }
}

bool InputManager::process_keyboard(const int& key, int& player_id) {
    bool is_key_mapped = true;

    /* Check the SDLKey values and move change the coords */
    switch ( key ) {
        case SDLK_w:
            player_id = 0;
            std::cout << "W was pressed" << std::endl;
            break;

        case SDLK_s:
            player_id = 0;
            std::cout << "S was pressed" << std::endl;
            break;

        case SDLK_a:
            player_id = 0;
            std::cout << "A was pressed" << std::endl;
            break;

        case SDLK_d:
            player_id = 0;
            std::cout << "D was pressed" << std::endl;
            break;

        case SDLK_t:
            player_id = 1;
            std::cout << "T was pressed" << std::endl;
            break;

        case SDLK_f:
            player_id = 1;
            std::cout << "F was pressed" << std::endl;
            break;

        case SDLK_g:
            player_id = 1;
            std::cout << "G was pressed" << std::endl;
            break;

        case SDLK_h:
            player_id = 1;
            std::cout << "H was pressed" << std::endl;
            break;

        case SDLK_i:
            player_id = 2;
            std::cout << "I was pressed" << std::endl;
            break;

        case SDLK_j:
            player_id = 2;
            std::cout << "J was pressed" << std::endl;
            break;

        case SDLK_k:
            player_id = 2;
            std::cout << "K was pressed" << std::endl;
            break;

        case SDLK_l:
            player_id = 2;
            std::cout << "L was pressed" << std::endl;
            break;

        case SDLK_SPACE:
            std::cout << "SPACE was pressed" << std::endl;
            break;

        case SDLK_LEFT:
            player_id = 3;
            std::cout << "Left was pressed" << std::endl;
            break;

        case SDLK_UP:
            player_id = 3;
            std::cout << "Up was pressed" << std::endl;
            break;

        case SDLK_DOWN:
            player_id = 3;
            std::cout << "Down was pressed" << std::endl;
            break;

        case SDLK_RIGHT:
            player_id = 3;
            std::cout << "Right was pressed" << std::endl;
            break;

        case SDLK_F5:
            std::cout << "F5 was pressed" << std::endl;
            break;

        default:
            is_key_mapped = false;
            std::cout << "UNMAPPED was pressed" << std::endl;
            break;
    }

    return is_key_mapped;
}

bool InputManager::process_controller_button(const int& button) {
    bool is_mapped_button = true;

    switch (button) {

        // LETTER BUTTONS

        case SDL_CONTROLLER_BUTTON_A:
            std::cout << "A button was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_B:
            std::cout << "B button was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_X:
            std::cout << "X button was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_Y:
            std::cout << "Y button was pressed" << std::endl;
            break;

        // SHOULDER BUTTONS

        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            std::cout << "Left bumper was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            std::cout << "Right bumper was pressed" << std::endl;
            break;

        // SPECIAL BUTTONS

        case SDL_CONTROLLER_BUTTON_START:
            std::cout << "Start button was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_BACK:
            std::cout << "Back button was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_GUIDE:
            std::cout << "Guide button was pressed" << std::endl;
            break;

        // DIRECTIONAL BUTTONS

        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            std::cout << "D-UP button was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            std::cout << "D-DOWN button was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            std::cout << "D-LEFT button was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            std::cout << "D-RIGHT button was pressed" << std::endl;
            break;

        // STICK BUTTONS

        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            std::cout << "Left stick button was pressed" << std::endl;
            break;

        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            std::cout << "Right stick button was pressed" << std::endl;
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
