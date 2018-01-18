#pragma once

#include <vector>

#include "EventSystem.h"
#include "SDL.h"


class InputManager : public EventSystem<InputManager> {

    const int DEADZONE = 8000; // Minimum range of displacement for joystick before reading event
    std::vector<SDL_GameController*> controllers;

public:
    InputManager();

    void process_input(SDL_Event* event);
    void quit();
};