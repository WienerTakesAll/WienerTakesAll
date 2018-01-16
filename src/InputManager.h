#pragma once

#include <vector>

#include "SDL.h"


class InputManager {

    std::vector<SDL_GameController*> controllers;

public:
    InputManager();

    void process_input(SDL_Event* event) const;
    void quit();
};