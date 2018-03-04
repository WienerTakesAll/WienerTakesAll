#pragma once
#include <array>

#include "UIObject.h"
#include "AssetManager.h"
#include "EventSystem.h"

class GameplayHud {
public:
    GameplayHud(AssetManager& asset_manager);
    void load();
    void render() const;
    void update_score(const int& player, const int& value);

private:
    AssetManager& asset_manager_;
    MeshAsset* square_mesh_;
    ShaderAsset* ui_shader_;

    UIObject scoreboard_;
    std::array<UIObject, 4> scores_;
    UIObject score_p1_;
    UIObject score_p2_;
    UIObject score_p3_;
    UIObject score_p4_;
};
