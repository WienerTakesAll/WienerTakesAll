#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "GameplayHud.h"
#include "Powerup.h"

namespace {
    const int MAX_SCORE = 2500;
    const std::string POWERUP_HOLDER_TEXTURE_PATH = "assets/textures/powerup_holder_ui.png";
    const std::string KETCHUP_TEXTURE_PATH = "assets/textures/ketchup_powerup_ui.png";
    const std::string MUSTARD_TEXTURE_PATH = "assets/textures/mustard_powerup_ui.png";
    const std::string RELISH_TEXTURE_PATH = "assets/textures/relish_powerup_ui.png";
}

GameplayHud::GameplayHud(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {
}

void GameplayHud::load() {
    square_mesh_ = asset_manager_.get_mesh_asset("assets/models/UIRect.obj");
    ui_shader_ = asset_manager_.get_shader_asset("assets/shaders/UIShader");
    TextureAsset* background_tex =
        asset_manager_.get_texture_asset("assets/textures/score_bg.png", true);
    scoreboard_ = UIObject(
                      glm::vec2(-0.25f, -0.33f),
                      glm::vec3(1.0f),
                      glm::vec2(0.5f, 0.66f),
                      square_mesh_,
                      background_tex,
                      ui_shader_
                  );

    TextureAsset* p1_tex = asset_manager_.get_texture_asset("assets/textures/score_1.png", true);
    UIObject score_p1 = UIObject(
                            glm::vec2(-0.25f, 0.0f),
                            glm::vec3(1.0f),
                            glm::vec2(0.25f, 0.33f),
                            square_mesh_,
                            p1_tex,
                            ui_shader_
                        );

    TextureAsset* p2_tex = asset_manager_.get_texture_asset("assets/textures/score_2.png", true);
    UIObject score_p2 = UIObject(
                            glm::vec2(0.0f),
                            glm::vec3(1.0f),
                            glm::vec2(0.25f, 0.33f),
                            square_mesh_,
                            p2_tex,
                            ui_shader_
                        );

    TextureAsset* p3_tex = asset_manager_.get_texture_asset("assets/textures/score_3.png", true);
    UIObject score_p3 = UIObject(
                            glm::vec2(-0.25f, -0.33f),
                            glm::vec3(1.0f),
                            glm::vec2(0.25f, 0.33f),
                            square_mesh_,
                            p3_tex,
                            ui_shader_
                        );

    TextureAsset* p4_tex = asset_manager_.get_texture_asset("assets/textures/score_4.png", true);
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
        score.set_scale(0.0f);
    }

    float pointer_size_x = 0.08f;
    float pointer_size_y = pointer_size_x * 16.f / 9.f;
    std::array<glm::vec2, 4> player_screen_centers = {
        // NOTE: 0.5 == quarter length of screen
        // (-0.5, 0.5f) center of top left quadrant
        glm::vec2(-0.5f - pointer_size_x / 2.f, 0.5f - pointer_size_y / 2.f),
        // (0.5, 0.5f) center of top right quadrant
        glm::vec2(0.5f - pointer_size_x / 2.f, 0.5f - pointer_size_y / 2.f),
        // (-0.5, -0.5f) center of bottom left quadrant
        glm::vec2(-0.5f - pointer_size_x / 2.f, -0.5f - pointer_size_y / 2.f),
        // (0.5, -0.5f) center of bottom right quadrant
        glm::vec2(0.5f - pointer_size_x / 2.f, -0.5f - pointer_size_y / 2)
    };


    for (unsigned int i = 0; i < it_pointers_.size(); i++) {
        TextureAsset* pointer_tex = asset_manager_.get_texture_asset("assets/textures/pointer.png");
        it_pointers_[i] = UIObject(
                              player_screen_centers[i],
                              glm::vec3(1.0f),
                              glm::vec2(pointer_size_x, pointer_size_y),
                              square_mesh_,
                              pointer_tex,
                              ui_shader_
                          );
        it_pointers_[i].visible_ = false;
    }

    // Pre-load powerups
    asset_manager_.get_texture_asset(KETCHUP_TEXTURE_PATH);
    asset_manager_.get_texture_asset(MUSTARD_TEXTURE_PATH);
    asset_manager_.get_texture_asset(RELISH_TEXTURE_PATH);

    TextureAsset* powerup_holder_tex = asset_manager_.get_texture_asset(POWERUP_HOLDER_TEXTURE_PATH);

    float holder_size_x = 0.10f;
    float holder_size_y = holder_size_x * 16.0f / 9.0f;
    std::array<glm::vec2, 4> player_holders_position = {
        glm::vec2(-1.0f + holder_size_x / 3.0, 1.0f - holder_size_y * 1.25f),
        glm::vec2(1.0 - holder_size_x * 1.25f, 1.0f - holder_size_y * 1.25f),
        glm::vec2(-1.0f + holder_size_x / 3.0, 0.0f - holder_size_y * 1.25f),
        glm::vec2(1.0 - holder_size_x * 1.25f, 0.0f - holder_size_y * 1.25f)
    };

    for (unsigned int i = 0; i < powerup_holders_.size(); i++) {
        powerup_holders_[i] = UIObject(
                                  player_holders_position[i],
                                  glm::vec3(1.0),
                                  glm::vec2(holder_size_x, holder_size_y),
                                  square_mesh_,
                                  powerup_holder_tex,
                                  ui_shader_
                              );
    }
}

void GameplayHud::render() const {
    scoreboard_.render(glm::mat4());

    for (auto& score : scores_) {
        score.render(glm::mat4());
    }

    for (unsigned int i = 0; i < it_pointers_.size(); i++) {
        it_pointers_[i].render(it_pointer_transforms_[i]);
    }

    for (auto& holder : powerup_holders_) {
        holder.render(glm::mat4());
    }
}

void GameplayHud::update_score(const int& player, const int& score) {
    scores_.at(player).set_scale(std::min((float) score / MAX_SCORE, 1.0f));
}

void GameplayHud::reset_scores() {
    for (auto& score : scores_) {
        score.set_scale(0.0f);
    }
}

void GameplayHud::update_it_pointer(int player_id, glm::vec3 vector_to_it) {
    if (current_it_ == player_id) {
        it_pointers_[player_id].visible_ = false;
        return;
    }

    it_pointers_[player_id].visible_ = true;

    glm::vec2 origin_translate = glm::normalize(glm::vec2(vector_to_it.x, vector_to_it.z));

    it_pointer_transforms_[player_id] =
        glm::translate(glm::mat4(), glm::vec3(origin_translate[0] / -4.f, origin_translate[1] / 4.f, 0.f));

    float angle = glm::orientedAngle(origin_translate, glm::vec2(0, 1));

    const float PI = 3.14159;

    // only show the pointer if it's behind the car
    if (angle > PI / 2 || angle < -PI / 2) {
        it_pointers_[player_id].set_rotation(angle);
    } else {
        it_pointers_[player_id].visible_ = false;
    }

}

void GameplayHud::new_it(int it_id) {
    current_it_ = it_id;
}

void GameplayHud::pickup_powerup(int player, int type) {
    if (type == PowerupType::KETCHUP) {
        powerup_holders_[player].set_texture(asset_manager_.get_texture_asset(KETCHUP_TEXTURE_PATH));
    } else if (type == PowerupType::MUSTARD) {
        powerup_holders_[player].set_texture(asset_manager_.get_texture_asset(MUSTARD_TEXTURE_PATH));
    } else if (type == PowerupType::RELISH) {
        powerup_holders_[player].set_texture(asset_manager_.get_texture_asset(RELISH_TEXTURE_PATH));
    }
}

void GameplayHud::use_powerup(int player) {
    powerup_holders_[player].set_texture(asset_manager_.get_texture_asset(POWERUP_HOLDER_TEXTURE_PATH));
}
