#pragma once

#include "UIObject.h"
#include "AssetManager.h"

class EndGameScreen {
public:
    EndGameScreen(AssetManager& asset_manager);
    void load();
    void set_winner(int winner_id);
    void set_no_winner();
    void render() const;

private:
    AssetManager& asset_manager_;

    MeshAsset* square_mesh_;
    ShaderAsset* ui_shader_;

    UIObject crown_;
    glm::mat4 crown_location_;
};
