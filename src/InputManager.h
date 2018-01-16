#pragma once

#include <vector>

#include "AudioSystem.h"
#include "SDL.h"


class InputManager {

    const int DEADZONE = 8000; // Minimum range of displacement for joystick before reading event
    std::vector<SDL_GameController*> controllers;
    AudioSystem audio_system;

public:
    InputManager();

    void process_input(SDL_Event* event) const;
    void quit();
};