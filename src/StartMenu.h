#pragma once

#include "UIObject.h"
#include "AssetManager.h"

class StartMenu {
public:
    StartMenu(AssetManager& asset_manager);
    void load();
    void move_selection_up();
    void move_selection_down();
    int selected_num_of_players();
    void render() const;

private:
    AssetManager& asset_manager_;

    MeshAsset* square_mesh_;
    ShaderAsset* ui_shader_;

    UIObject background_;
    UIObject logo_;
    UIObject one_players_;
    UIObject two_players_;
    UIObject three_players_;
    UIObject four_players_;
    std::vector<UIObject> selection_indicators_;
    unsigned int active_selection;
};
