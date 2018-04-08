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
    void new_it(int it_id);
    void pickup_powerup(int player, int type);
    void use_powerup(int player);
    void reset_powerups();

    void set_num_ai(int num_ai);

private:
    AssetManager& asset_manager_;
    MeshAsset* square_mesh_;
    ShaderAsset* ui_shader_;

    UIObject scoreboard_;
    std::array<UIObject, 4> scores_;
    int current_it_;

    std::array<UIObject, 4> it_pointers_;
    std::array<glm::mat4, 4> it_pointer_transforms_;

    std::array<UIObject, 4> powerup_holders_;
    int num_ai_;
};
