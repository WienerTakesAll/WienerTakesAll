#pragma once

#include <vector>

#include "SDL.h"


class InputManager {

    const int MAX_PLAYERS = 4; // Maximum number of players in a game
    const int DEADZONE = 8000; // Minimum range of displacement for joystick before reading event
    std::vector<SDL_GameController*> controllers;

public:
    InputManager();

    void process_input(SDL_Event* event) const;
    void quit();
};