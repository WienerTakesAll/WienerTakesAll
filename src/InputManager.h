#pragma once

#include <array>

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
    std::array<SDL_GameController*, 4> controllers_;
    std::array<SDL_Haptic*, 4> haptics_;
    int num_players_;

    void handle_load_event(const Event& e);
    void handle_reload_settings_event(const Event& event);
    void handle_vehicle_collision(const Event& e);
    void handle_new_game_state(const Event& e);
    void handle_dominate_controls(const Event& e);
    void handle_reverse_controls(const Event& e);

    bool process_keyboard(const int& key, int& player_id);
    bool process_controller_button(const int& button);
    bool process_controller_axis(const int& axis, const int& value);

    void rumble_controller(const int id) const;

    // Returns appropriate player_id from SDL2 Joystick index
    const int get_player_id_from_joystick_index(const int joystick_index) const;

    std::array<SDL_GameControllerButton, 11> buttons_;
    std::array<SDL_GameControllerAxis, 4> axes_;
    std::array<SDL_Keycode, 27> keys_;
};
