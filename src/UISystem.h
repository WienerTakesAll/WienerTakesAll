#pragma once

#include <vector>

#include "GL/glew.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include "EventSystem.h"
#include "Shader.h"
#include "UIObject.h"
#include "MeshAsset.h"
#include "GameState.h"
#include "StartMenu.h"
#include "LoadingScreen.h"
#include "GameplayHud.h"
#include "EndGameScreen.h"

class AssetManager;

class UISystem : public EventSystem<UISystem> {
public:
    UISystem(AssetManager& asset_manager);
    void update();
    void render() const;

private:
    void start_render() const;
    void end_render() const;

    void handle_load(const Event& e);
    void handle_key_press(const Event& e);
    void handle_new_game_state(const Event& e);
    void handle_update_score(const Event& e);
    void handle_major_load_complete(const Event& e);
    void handle_update_direction_to_it(const Event& e);
    void handle_new_it(const Event& e);

    AssetManager& asset_manager_;
    int loading_frames_counter_;
    StartMenu start_menu_;
    LoadingScreen loading_screen_;
    GameplayHud gameplay_hud_;
    EndGameScreen end_game_screen_;
    SDL_Window* window_;
    GameState current_game_state_;

    //Checks whether joystick has already been pressed up/down
    bool controller_buffer_;
};
