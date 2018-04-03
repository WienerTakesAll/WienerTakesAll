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
    std::vector<SDL_Haptic*> haptics_;

    void handle_load_event(const Event& e);
    void handle_reload_settings_event(const Event& event);
    void handle_vehicle_collision(const Event& e);

    bool process_keyboard(const int& key, int& player_id);
    bool process_controller_button(const int& button);
    bool process_controller_axis(const int& axis, const int& value);

    void rumble_controller(const int id) const;

	// Returns appropriate player_id from SDL2 Joystick index
	const int get_player_id_from_joystick_index(const int joystick_index) const;
};
