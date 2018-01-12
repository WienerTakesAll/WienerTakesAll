#ifndef INPUT_H
#define INPUT_H

#include <vector>

#include "SDL.h"


class InputManager {

    std::vector<SDL_GameController*> controllers;

public:
    InputManager();

    int pId;
    void ProcessInput(SDL_Event* event) const;
};

#endif