#include "GameplayHud.h"
#include <glm/gtc/matrix_transform.hpp>

GameplayHud::GameplayHud(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {
}

void GameplayHud::load() {
    square_mesh_ = asset_manager_.get_mesh_asset("assets/models/UIRect.obj");
    ui_shader_ = asset_manager_.get_shader_asset("assets/shaders/UIShader");
    TextureAsset* background_tex =
        asset_manager_.get_texture_asset("assets/textures/score_bg.png");
    scoreboard_ = UIObject(
                      glm::vec2(-0.25f, -0.33f),
                      glm::vec3(1.0f),
                      glm::vec2(0.5f, 0.66f),
                      square_mesh_,
                      background_tex,
                      ui_shader_
                  );

    // TODO: wrap these into an array or something
    TextureAsset* p1_tex = asset_manager_.get_texture_asset("assets/textures/score_1.png");
    score_p1_ = UIObject(
                    glm::vec2(-0.25f, 0.0f),
                    glm::vec3(1.0f),
                    glm::vec2(0.25f, 0.33f),
                    square_mesh_,
                    p1_tex,
                    ui_shader_
                );

    TextureAsset* p2_tex = asset_manager_.get_texture_asset("assets/textures/score_2.png");
    score_p2_ = UIObject(
                    glm::vec2(0.0f),
                    glm::vec3(1.0f),
                    glm::vec2(0.25f, 0.33f),
                    square_mesh_,
                    p2_tex,
                    ui_shader_
                );

    TextureAsset* p3_tex = asset_manager_.get_texture_asset("assets/textures/score_3.png");
    score_p3_ = UIObject(
                    glm::vec2(-0.25f, -0.33f),
                    glm::vec3(1.0f),
                    glm::vec2(0.25f, 0.33f),
                    square_mesh_,
                    p3_tex,
                    ui_shader_
                );

    TextureAsset* p4_tex = asset_manager_.get_texture_asset("assets/textures/score_4.png");
    score_p4_ = UIObject(
                    glm::vec2(0.0f, -0.33f),
                    glm::vec3(1.0f),
                    glm::vec2(0.25f, 0.33f),
                    square_mesh_,
                    p4_tex,
                    ui_shader_
                );
    // score_p1_.scale(0.25f);
    // score_p2_.scale(0.50f);
    // score_p3_.scale(0.75f);
    // score_p4_.scale(1.00f);
}

void GameplayHud::render() const {
    scoreboard_.render(glm::mat4());
    score_p1_.render(glm::mat4());
    score_p2_.render(glm::mat4());
    score_p3_.render(glm::mat4());
    score_p4_.render(glm::mat4());

    // for (auto score : scores_) {
    //     score.render(glm::mat4());
    // }
}

// void GameplayHud::handle_score_update(const Event& event) {
//     int player_id = event.get<int>("player_id", true).first;
//     int value = event.get<int>("value", true).first;
// }
