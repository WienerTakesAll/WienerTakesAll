#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include "GameplayHud.h"

namespace {
    const int MAX_SCORE = 2500;
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

    float pointer_size_x = 0.16f;
    float pointer_size_y = 0.22f;
    std::array<glm::vec2, 4> player_screen_centers = {
        glm::vec2(-0.5f - pointer_size_x / 2.f, 0.5f - pointer_size_y / 2.f),
        glm::vec2(0.5f - pointer_size_x / 2.f, 0.5f - pointer_size_y / 2.f),
        glm::vec2(-0.5f - pointer_size_x / 2.f, -0.5f - pointer_size_y / 2.f),
        glm::vec2(0.5f - pointer_size_x / 2.f, -0.5f - pointer_size_y / 2)
    };


    for (int i = 0; i < leader_pointers_.size(); i++) {
        TextureAsset* pointer_tex = asset_manager_.get_texture_asset("assets/textures/logo.png");
        leader_pointers_[i] = UIObject(
                                  player_screen_centers[i],
                                  glm::vec3(1.0f),
                                  glm::vec2(0.16f, 0.22f),
                                  square_mesh_,
                                  pointer_tex,
                                  ui_shader_
                              );
    }

}

void GameplayHud::render() const {
    scoreboard_.render(glm::mat4());

    for (auto& score : scores_) {
        score.render(glm::mat4());
    }

    for (int i = 0; i < leader_pointers_.size(); i++) {
        leader_pointers_[i].render(leader_pointer_positions_[i]);
    }
}

void GameplayHud::update_score(const int& player, const int& score) {
    scores_.at(player).scale(std::min((float) score / MAX_SCORE, 1.0f));
}

void GameplayHud::reset_scores() {
    for (auto& score : scores_) {
        score.scale(0.0f);
    }
}

void GameplayHud::update_leader_pointer(int player_id, std::array<float, 3> vector_to_leader) {
    glm::vec2 origin_translate = glm::normalize(glm::vec2(vector_to_leader[0], vector_to_leader[2]));
    std::cout << "position vector " << player_id  << ": "
              << origin_translate[0] << " "
              << origin_translate[1] << " " << std::endl;
    leader_pointer_positions_[player_id] =
        glm::translate(glm::mat4(), glm::vec3(origin_translate[0] / -4.f, origin_translate[1] / 4.f, 0.f));
}




















