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
    void reset_scores();
    void update_it_pointer(int player_id, glm::vec3 vector_to_it);

private:
    AssetManager& asset_manager_;
    MeshAsset* square_mesh_;
    ShaderAsset* ui_shader_;

    UIObject scoreboard_;
    std::array<UIObject, 4> scores_;

    std::array<UIObject, 4> leader_pointers_;
    std::array<glm::mat4, 4> leader_pointer_positions_;
};
