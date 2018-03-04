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
#include "GameplayHud.h"

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

    AssetManager& asset_manager_;
    StartMenu start_menu_;
    GameplayHud gameplay_hud_;
    SDL_Window* window_;
    GameState current_game_state_;

    //Checks whether joystick has already been pressed up/down
    bool controller_buffer_;
};
