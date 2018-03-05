#include <iostream>

#include "glm/gtc/matrix_transform.hpp"

#include "EndGameScreen.h"

EndGameScreen::EndGameScreen(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {

}

void EndGameScreen::load() {
    square_mesh_ = asset_manager_.get_mesh_asset("assets/models/UIRect.obj");
    ui_shader_ = asset_manager_.get_shader_asset("assets/shaders/UIShader");
    TextureAsset* crown_tex =
        asset_manager_.get_texture_asset("assets/textures/crown.png");
    crown_ = UIObject(
                 glm::vec2(0.f),
                 glm::vec3(1.0f),
                 glm::vec2(0.3f),
                 square_mesh_,
                 crown_tex,
                 ui_shader_
             );
    crown_.visible_ = false;
}

void EndGameScreen::set_winner(int winner_id) {
    crown_.visible_ = true;
}

void EndGameScreen::set_no_winner() {
    set_winner(0); // TODO
}

void EndGameScreen::render() const {
    crown_.render(crown_location_);
}

