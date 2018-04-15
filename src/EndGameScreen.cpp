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

    TextureAsset* background_tex =
        asset_manager_.get_texture_asset("assets/textures/greyed_out.png");
    background_ = UIObject( // cover screen
                      glm::vec2(-1.f),
                      glm::vec3(1.0f),
                      glm::vec2(2.0f),
                      square_mesh_,
                      background_tex,
                      ui_shader_
                  );

    TextureAsset* congratulations_tex =
        asset_manager_.get_texture_asset("assets/textures/congratulations.png");
    congratulations_ = UIObject( // cover screen
                           glm::vec2(-1.f),
                           glm::vec3(1.0f),
                           glm::vec2(2.0f),
                           square_mesh_,
                           congratulations_tex,
                           ui_shader_
                       );
}

void EndGameScreen::set_winner(int winner_id) {
    crown_.visible_ = true;

    congratulations_.visible_ = true;

    switch (winner_id) {
        case 0:
            crown_location_ = // top left
                glm::translate(glm::mat4(), glm::vec3(-0.8f, 0.6f, 0.f));
            break;

        case 1:
            crown_location_ = // top right
                glm::translate(glm::mat4(), glm::vec3(0.2f, 0.6f, 0.f));
            break;

        case 2:
            crown_location_ = // bottom left
                glm::translate(glm::mat4(), glm::vec3(-0.8f, -0.45f, 0.f));
            break;

        case 3:
            crown_location_ = // bottom right
                glm::translate(glm::mat4(), glm::vec3(0.2f, -0.45f, 0.f));
            break;

        default:
            crown_location_ = // not visible
                glm::translate(glm::mat4(), glm::vec3(1000, 1000, 1000));
            congratulations_.visible_ = false;
            break;
    }
}

void EndGameScreen::render() const {
    background_.render(glm::mat4());
    congratulations_.render(glm::mat4());
    crown_.render(crown_location_);
}

