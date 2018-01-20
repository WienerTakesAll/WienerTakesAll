#pragma once

#include <vector>

#include "EventSystem.h"
#include "SDL.h"


class InputManager : public EventSystem<InputManager> {
public:
    InputManager();

    void process_input(SDL_Event* event);
    void quit();

public:
    std::vector<SDL_GameController*> controllers_;
};
