#pragma once

#include <vector>
#include <memory>

#include "EventSystem.h"
#include "SDL.h"

struct InputSettings;

class InputManager : public EventSystem<InputManager> {
public:
    InputManager(const InputSettings& settings);

    void process_input(SDL_Event* event);
    void quit();

private:
    const InputSettings& settings_;
    std::vector<SDL_GameController*> controllers_;

    void handle_load_event(const Event& e);
    void handle_reload_settings_event(const Event& event);
};
