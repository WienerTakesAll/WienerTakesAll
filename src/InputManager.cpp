#include <iostream>
#include <vector>
#include "InputManager.h"
#include "SDL.h"

const int MAX_PLAYERS = 4;

InputManager::InputManager() {
    int num_controllers_player = SDL_NumJoysticks();

    // Link player controllers
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_GameControllerEventState(SDL_ENABLE);

    SDL_Joystick* joystick = nullptr;
    SDL_GameController* controller = nullptr;

    for (int pId = 0; pId < num_controllers_player; ++pId) {
        joystick = SDL_JoystickOpen(pId);

        if (SDL_IsGameController(pId)) {
            controller = SDL_GameControllerOpen(pId);

            if (controller) {
                controllers.push_back(controller);
                std::cout << "Controller for player " << pId << " linked" << std::endl;
            } else {
                fprintf(stderr, "Could not open gamecontroller %i: %s\n", pId, SDL_GetError());
            }

            break;
        }
    }

    // Create AI controllers
    if (num_controllers_player < MAX_PLAYERS) {
        for (int pId = num_controllers_player; pId < MAX_PLAYERS; ++pId) {
            // Create AI controller
            std::cout << "AI Controller for player " << pId << " created" << std::endl;
        }
    }
}

void InputManager::ProcessInput(SDL_Event* event) const {
    switch (event->type) {
        case SDL_KEYDOWN:

            /* Check the SDLKey values and move change the coords */
            switch ( event->key.keysym.sym ) {
                case SDLK_LEFT:
                    std::cout << "Left was pressed" << std::endl;
                    break;

                default:
                    break;
            }

        case SDL_CONTROLLERBUTTONDOWN:

            switch (event->cbutton.button) {

                case SDL_CONTROLLER_BUTTON_A:
                    std::cout << "A button was pressed" << std::endl;
                    break;

                default:
                    break;
            }

            break;

        case SDL_JOYAXISMOTION:
            int deadzone = 3200; // Range of displacement for joystick before reading event

            if ((event->jaxis.value < -deadzone) || (event->jaxis.value > deadzone)) {
                // Left-right movement
                if (event->jaxis.value < 0) {
                    std::cout << "Move left" << std::endl;
                } else if (event->jaxis.value > 0) {
                    std::cout << "Move right" << std::endl;
                }
            }

            if (event->jaxis.axis == 1) {
                // Up-down movement
            }

            break;

    }
}

