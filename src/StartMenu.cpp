#include <iostream>

#include "StartMenu.h"

namespace {

}

StartMenu::StartMenu(AssetManager& asset_manager)
    : asset_manager_(asset_manager)
    , active_selection(0) {
}

void StartMenu::load() {
    square_mesh_ = asset_manager_.get_mesh_asset("assets/models/UIRect.obj");
    ui_shader_ = asset_manager_.get_shader_asset("assets/shaders/UIShader");
    TextureAsset* background_tex =
        asset_manager_.get_texture_asset("assets/textures/backyard_bbq.png");
    background_ = UIObject( // cover screen
                      glm::vec2(-1.f),
                      glm::vec3(1.0f),
                      glm::vec2(2.0f),
                      square_mesh_,
                      background_tex,
                      ui_shader_
                  );

    TextureAsset* logo_tex =
        asset_manager_.get_texture_asset("assets/textures/logo.png");
    logo_ = UIObject( // top middle
                glm::vec2(-0.4f, .1f),
                glm::vec3(1.0f),
                glm::vec2(0.8f),
                square_mesh_,
                logo_tex,
                ui_shader_
            );

    TextureAsset* one_players_tex =
        asset_manager_.get_texture_asset("assets/textures/one_players.png");
    one_players_ = UIObject( // top middle
                       glm::vec2(-.475f, -0.55f),
                       glm::vec3(1.0f),
                       glm::vec2(.95f),
                       square_mesh_,
                       one_players_tex,
                       ui_shader_
                   );

    TextureAsset* two_players_tex =
        asset_manager_.get_texture_asset("assets/textures/two_players.png");
    two_players_ = UIObject( // top middle
                       glm::vec2(-.475f, -.8f),
                       glm::vec3(1.0f),
                       glm::vec2(.95f),
                       square_mesh_,
                       two_players_tex,
                       ui_shader_
                   );

    TextureAsset* three_players_tex =
        asset_manager_.get_texture_asset("assets/textures/three_players.png");
    three_players_ = UIObject( // top middle
                         glm::vec2(-.475f, -1.05f),
                         glm::vec3(1.0f),
                         glm::vec2(.95f),
                         square_mesh_,
                         three_players_tex,
                         ui_shader_
                     );

    TextureAsset* four_players_tex =
        asset_manager_.get_texture_asset("assets/textures/four_players.png");
    four_players_ = UIObject( // top middle
                        glm::vec2(-.475f, -1.3f),
                        glm::vec3(1.0f),
                        glm::vec2(.95f),
                        square_mesh_,
                        four_players_tex,
                        ui_shader_
                    );


    TextureAsset* unselected_tex =
        asset_manager_.get_texture_asset("assets/textures/raw_sausage.png");

    for (int i = 0; i < 4; i++) { // 4 selections
        selection_indicators_.emplace_back(
            glm::vec2(), // garbage
            glm::vec3(1.0f),
            glm::vec2(1.15f),
            square_mesh_,
            unselected_tex,
            ui_shader_
        );
    }


    TextureAsset* selected_tex =
        asset_manager_.get_texture_asset("assets/textures/grilled_sausage.png");
    selection_indicators_[active_selection].set_texture(selected_tex);

    selection_indicators_[0].set_origin(glm::vec2(-.5f, -0.5f));
    selection_indicators_[1].set_origin(glm::vec2(-.5f, -.72f));
    selection_indicators_[2].set_origin(glm::vec2(-.5f, -.94f));
    selection_indicators_[3].set_origin(glm::vec2(-.5f, -1.16f));

}

void StartMenu::move_selection_up() {
    if (active_selection == 0) {
        return;
    }

    TextureAsset* unselected_tex =
        asset_manager_.get_texture_asset("assets/textures/raw_sausage.png");
    selection_indicators_[active_selection].set_texture(unselected_tex);

    active_selection--;

    TextureAsset* selected_tex =
        asset_manager_.get_texture_asset("assets/textures/grilled_sausage.png");
    selection_indicators_[active_selection].set_texture(selected_tex);
}

void StartMenu::move_selection_down() {
    if (active_selection >= selection_indicators_.size() - 1) {
        return;
    }

    TextureAsset* unselected_tex =
        asset_manager_.get_texture_asset("assets/textures/raw_sausage.png");
    selection_indicators_[active_selection].set_texture(unselected_tex);

    active_selection++;

    TextureAsset* selected_tex =
        asset_manager_.get_texture_asset("assets/textures/grilled_sausage.png");
    selection_indicators_[active_selection].set_texture(selected_tex);
}

int StartMenu::selected_num_of_players() {
    return active_selection + 1;
}

void StartMenu::render() const {
    background_.render(glm::mat4());
    logo_.render(glm::mat4());

    for (auto selection_indicator : selection_indicators_) {
        selection_indicator.render(glm::mat4());
    }

    one_players_.render(glm::mat4());
    two_players_.render(glm::mat4());
    three_players_.render(glm::mat4());
    four_players_.render(glm::mat4());
}

