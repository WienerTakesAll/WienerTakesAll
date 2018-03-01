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

class AssetManager;

class UISystem : public EventSystem<UISystem> {
public:
    UISystem(AssetManager&);
    void update();
    void render() const;

private:
    void start_render() const;
    void end_render() const;

    void handle_load(const Event& e);
    void handle_key_press(const Event& e);

    AssetManager& asset_manager_;

    MeshAsset* square_mesh_;
    ShaderAsset* ui_shader_;

    UIObject start_bg_;
    UIObject logo_;
    UIObject hit_enter_or_start_;

    SDL_Window* window_;

    GameState current_game_state_;
};
