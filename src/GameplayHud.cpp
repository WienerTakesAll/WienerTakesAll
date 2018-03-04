#include "GameplayHud.h"
#include <glm/gtc/matrix_transform.hpp>

namespace {
    const float MAX_SCORE = 1000.0f;
}

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

    TextureAsset* p1_tex = asset_manager_.get_texture_asset("assets/textures/score_1.png");
    UIObject score_p1 = UIObject(
                            glm::vec2(-0.25f, 0.0f),
                            glm::vec3(1.0f),
                            glm::vec2(0.25f, 0.33f),
                            square_mesh_,
                            p1_tex,
                            ui_shader_
                        );

    TextureAsset* p2_tex = asset_manager_.get_texture_asset("assets/textures/score_2.png");
    UIObject score_p2 = UIObject(
                            glm::vec2(0.0f),
                            glm::vec3(1.0f),
                            glm::vec2(0.25f, 0.33f),
                            square_mesh_,
                            p2_tex,
                            ui_shader_
                        );

    TextureAsset* p3_tex = asset_manager_.get_texture_asset("assets/textures/score_3.png");
    UIObject score_p3 = UIObject(
                            glm::vec2(-0.25f, -0.33f),
                            glm::vec3(1.0f),
                            glm::vec2(0.25f, 0.33f),
                            square_mesh_,
                            p3_tex,
                            ui_shader_
                        );

    TextureAsset* p4_tex = asset_manager_.get_texture_asset("assets/textures/score_4.png");
    UIObject score_p4 = UIObject(
                            glm::vec2(0.0f, -0.33f),
                            glm::vec3(1.0f),
                            glm::vec2(0.25f, 0.33f),
                            square_mesh_,
                            p4_tex,
                            ui_shader_
                        );

    scores_ = { score_p1, score_p2, score_p3, score_p4 };

    for (auto& score : scores_) {
        score.scale(0.0f);
    }
}

void GameplayHud::render() const {
    scoreboard_.render(glm::mat4());

    for (auto& score : scores_) {
        score.render(glm::mat4());
    }
}

void GameplayHud::update_score(const int& player, const int& score) {
    scores_.at(player).scale(score / MAX_SCORE);
}
