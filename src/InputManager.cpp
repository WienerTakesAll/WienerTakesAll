#include <iostream>
#include <vector>
#include "InputManager.h"
#include "SDL.h"

InputManager::InputManager() {
    int num_controllers_player = SDL_NumJoysticks();

    // Link player controllers
    SDL_GameControllerEventState(SDL_ENABLE);
    SDL_GameController* controller = nullptr;

    for (int player_id = 0; player_id < num_controllers_player; ++player_id) {
        if (SDL_IsGameController(player_id)) {
            controller = SDL_GameControllerOpen(player_id);

            if (controller) {
                controllers.push_back(controller);
                std::cout << "Controller for player " << player_id << " linked" << std::endl;
            } else {
                fprintf(stderr, "Could not open gamecontroller %i: %s\n", player_id, SDL_GetError());
            }

        }
    }

    // Create AI controllers
    if (num_controllers_player < MAX_PLAYERS) {
        for (int player_id = num_controllers_player; player_id < MAX_PLAYERS; ++player_id) {
            // Create AI controller
            std::cout << "Create AI Controller for player " << player_id << " here" << std::endl;
        }
    }
}

void InputManager::process_input(SDL_Event* event) const {
    int player_id = -1;

    switch (event->type) {
        // Keyboard (Player 1)
        case SDL_KEYDOWN:
            player_id = 0;
            std::cout << "[Player " << player_id << "] ";

            /* Check the SDLKey values and move change the coords */
            switch ( event->key.keysym.sym ) {
                case SDLK_w:
                    std::cout << "W was pressed" << std::endl;
                    break;

                case SDLK_s:
                    std::cout << "S was pressed" << std::endl;
                    break;

                case SDLK_a:
                    std::cout << "A was pressed" << std::endl;
                    break;

                case SDLK_d:
                    std::cout << "D was pressed" << std::endl;
                    break;

                case SDLK_SPACE:
                    std::cout << "SPACE was pressed" << std::endl;
                    break;

                default:
                    break;
            }

            break;

        case SDL_CONTROLLERBUTTONDOWN:

            player_id = event->cbutton.which; // Joystick ID of event sender
            std::cout << "[Player " << player_id << "] ";

            switch (event->cbutton.button) {

                /**
                 * LETTER BUTTONS
                 */
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

                /**
                 * SHOULDER BUTTONS
                 */

                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                    std::cout << "Left bumper was pressed" << std::endl;
                    break;

                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                    std::cout << "Right bumper was pressed" << std::endl;
                    break;

                /**
                 * SPECIAL BUTTONS
                 */

                case SDL_CONTROLLER_BUTTON_START:
                    std::cout << "Start button was pressed" << std::endl;
                    break;

                case SDL_CONTROLLER_BUTTON_BACK:
                    std::cout << "Back button was pressed" << std::endl;
                    break;

                case SDL_CONTROLLER_BUTTON_GUIDE:
                    std::cout << "Guide button was pressed" << std::endl;
                    break;

                /**
                 * DIRECTIONAL BUTTONS
                 */

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

                /**
                 * STICK BUTTONS
                 */

                case SDL_CONTROLLER_BUTTON_LEFTSTICK:
                    std::cout << "Left stick button was pressed" << std::endl;
                    break;

                case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
                    std::cout << "Right stick button was pressed" << std::endl;
                    break;

                /**
                 * MISC
                 */

                case SDL_CONTROLLER_BUTTON_INVALID:
                    std::cout << "Invalid button was pressed" << std::endl;
                    break;

                default:
                    break;
            }

            break;

        case SDL_CONTROLLERAXISMOTION:
            int value = event->caxis.value; // Current displacement of joystick

            if ((value > -DEADZONE) && (value < DEADZONE)) {
                break;
            }

            player_id = event->caxis.which; // Joystick ID of event sender
            std::cout << "[Player " << player_id << "] ";

            switch (event->caxis.axis) {

                /**
                 * LEFT STICK
                 */

                case SDL_CONTROLLER_AXIS_LEFTX:
                    std::cout << "Left axis horizontal";
                    break;

                case SDL_CONTROLLER_AXIS_LEFTY:
                    std::cout << "Left axis vertical";
                    break;

                /**
                 * RIGHT STICK
                 */

                case SDL_CONTROLLER_AXIS_RIGHTX:
                    std::cout << "Right axis horizontal";
                    break;

                case SDL_CONTROLLER_AXIS_RIGHTY:
                    std::cout << "Right axis vertical";
                    break;

                /**
                 * TRIGGERS
                 */

                case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                    std::cout << "Left trigger";
                    break;

                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                    std::cout << "Right trigger";
                    break;

                /**
                 * MISC
                 */

                case SDL_CONTROLLER_AXIS_INVALID:
                    std::cout << "Invalid axis";
                    break;

                default:
                    break;
            }

            std::cout << " value: " << value << std::endl;

            break;

    }
}

void InputManager::quit() {
    for (SDL_GameController* game_controller : controllers) {
        SDL_GameControllerClose(game_controller);
    }

    controllers.clear();
    std::cout << "All controllers closed" << std::endl;
}