#include <iostream>

#include "StartMenu.h"

namespace {
    const float MENU_ITEM_SCALE = 0.7f;
    const float MENU_TEXT_X_POS = -0.35f;
    const float MENU_TEXT_Y_BASE = -0.41f;
    const float MENU_TEXT_SPACING = -0.2f;
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

    float menu_y[5] = { MENU_TEXT_Y_BASE };

    for (int i = 1; i < 5; ++i) {
        menu_y[i] = menu_y[i - 1] + MENU_TEXT_SPACING;
    }

    TextureAsset* one_players_tex =
        asset_manager_.get_texture_asset("assets/textures/one_players_selected.png"); // default selection
    asset_manager_.get_texture_asset("assets/textures/one_players.png"); // preload
    one_players_ = UIObject( // top middle
                       glm::vec2(MENU_TEXT_X_POS, menu_y[0]),
                       glm::vec3(1.0f),
                       glm::vec2(MENU_ITEM_SCALE),
                       square_mesh_,
                       one_players_tex,
                       ui_shader_
                   );

    TextureAsset* two_players_tex =
        asset_manager_.get_texture_asset("assets/textures/two_players.png");
    asset_manager_.get_texture_asset("assets/textures/two_players_selected.png"); // preload
    two_players_ = UIObject( // top middle
                       glm::vec2(MENU_TEXT_X_POS, menu_y[1]),
                       glm::vec3(1.0f),
                       glm::vec2(MENU_ITEM_SCALE),
                       square_mesh_,
                       two_players_tex,
                       ui_shader_
                   );

    TextureAsset* three_players_tex =
        asset_manager_.get_texture_asset("assets/textures/three_players.png");
    asset_manager_.get_texture_asset("assets/textures/three_players_selected.png"); // preload
    three_players_ = UIObject( // top middle
                         glm::vec2(MENU_TEXT_X_POS, menu_y[2]),
                         glm::vec3(1.0f),
                         glm::vec2(MENU_ITEM_SCALE),
                         square_mesh_,
                         three_players_tex,
                         ui_shader_
                     );

    TextureAsset* four_players_tex =
        asset_manager_.get_texture_asset("assets/textures/four_players.png");
    asset_manager_.get_texture_asset("assets/textures/four_players_selected.png"); // preload
    four_players_ = UIObject( // top middle
                        glm::vec2(MENU_TEXT_X_POS, menu_y[3]),
                        glm::vec3(1.0f),
                        glm::vec2(MENU_ITEM_SCALE),
                        square_mesh_,
                        four_players_tex,
                        ui_shader_
                    );

    TextureAsset* exit_game_tex =
        asset_manager_.get_texture_asset("assets/textures/exit_game.png");
    asset_manager_.get_texture_asset("assets/textures/exit_game_selected.png"); // preload
    exit_game_ = UIObject( // top middle
                     glm::vec2(MENU_TEXT_X_POS, menu_y[4]),
                     glm::vec3(1.0f),
                     glm::vec2(MENU_ITEM_SCALE),
                     square_mesh_,
                     exit_game_tex,
                     ui_shader_
                 );


    TextureAsset* unselected_tex =
        asset_manager_.get_texture_asset("assets/textures/raw_sausage.png");

    for (int i = 0; i < 5; i++) { // 4 selections
        selection_indicators_.emplace_back(
            glm::vec2(), // garbage
            glm::vec3(1.0f),
            glm::vec2(0.85f),
            square_mesh_,
            unselected_tex,
            ui_shader_
        );
    }


    TextureAsset* selected_tex =
        asset_manager_.get_texture_asset("assets/textures/grilled_sausage.png");
    selection_indicators_[active_selection].set_texture(selected_tex);

    selection_indicators_[0].set_origin(glm::vec2(-.5f, -0.55f));
    selection_indicators_[1].set_origin(glm::vec2(-.5f, -.79f));
    selection_indicators_[2].set_origin(glm::vec2(-.5f, -1.03f));
    selection_indicators_[3].set_origin(glm::vec2(-.5f, -1.27f));
    selection_indicators_[4].set_origin(glm::vec2(-.5f, -1.50f));
}

void StartMenu::move_selection_up() {
    if (active_selection == 0) {
        return;
    }

    TextureAsset* unselected_tex =
        asset_manager_.get_texture_asset("assets/textures/raw_sausage.png");
    selection_indicators_[active_selection].set_texture(unselected_tex);

    switch (active_selection) {
        case 0:
            one_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/one_players.png"));
            break;

        case 1:
            two_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/two_players.png"));
            break;

        case 2:
            three_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/three_players.png"));
            break;

        case 3:
            four_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/four_players.png"));
            break;

        case 4:
            exit_game_.set_texture(asset_manager_.get_texture_asset("assets/textures/exit_game.png"));
            break;
    }

    active_selection--;

    TextureAsset* selected_tex =
        asset_manager_.get_texture_asset("assets/textures/grilled_sausage.png");
    selection_indicators_[active_selection].set_texture(selected_tex);

    switch (active_selection) {
        case 0:
            one_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/one_players_selected.png"));
            break;

        case 1:
            two_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/two_players_selected.png"));
            break;

        case 2:
            three_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/three_players_selected.png"));
            break;

        case 3:
            four_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/four_players_selected.png"));
            break;

        case 4:
            exit_game_.set_texture(asset_manager_.get_texture_asset("assets/textures/exit_game_selected.png"));
            break;
    }
}

void StartMenu::move_selection_down() {
    if (active_selection >= selection_indicators_.size() - 1) {
        return;
    }

    TextureAsset* unselected_tex =
        asset_manager_.get_texture_asset("assets/textures/raw_sausage.png");
    selection_indicators_[active_selection].set_texture(unselected_tex);

    switch (active_selection) {
        case 0:
            one_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/one_players.png"));
            break;

        case 1:
            two_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/two_players.png"));
            break;

        case 2:
            three_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/three_players.png"));
            break;

        case 3:
            four_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/four_players.png"));
            break;

        case 4:
            exit_game_.set_texture(asset_manager_.get_texture_asset("assets/textures/exit_game.png"));
            break;
    }

    active_selection++;

    TextureAsset* selected_tex =
        asset_manager_.get_texture_asset("assets/textures/grilled_sausage.png");
    selection_indicators_[active_selection].set_texture(selected_tex);

    switch (active_selection) {
        case 0:
            one_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/one_players_selected.png"));
            break;

        case 1:
            two_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/two_players_selected.png"));
            break;

        case 2:
            three_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/three_players_selected.png"));
            break;

        case 3:
            four_players_.set_texture(asset_manager_.get_texture_asset("assets/textures/four_players_selected.png"));
            break;

        case 4:
            exit_game_.set_texture(asset_manager_.get_texture_asset("assets/textures/exit_game_selected.png"));
            break;
    }
}

int StartMenu::selected_num_of_players() {
    return active_selection + 1;
}

bool StartMenu::selected_exit() {
    return active_selection == 4;
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
    exit_game_.render(glm::mat4());
}

