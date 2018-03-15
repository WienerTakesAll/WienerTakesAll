#pragma once

#include <glm/glm.hpp>
#include <map>

#include "GameState.h"
#include "Powerup.h"

class PowerupSubsystem {
public:
    PowerupSubsystem();
    void load();
    void update();
    void set_new_game_state(const GameState new_game_state);
    void create_powerup(const int object_id, const PowerupType type, glm::vec3 pos);

    // Modifiers
    void change_powerup_position(const int object_id, glm::vec3 pos);
    void change_powerup_type(const PowerupType new_type);
    void pickup_powerup(const int object_id);
    PowerupType use_powerup(const int object_id);

    // Utility functions
    const bool can_use_powerup(const int object_id) const;
    const bool is_powerup(const int object_id) const;
    glm::vec3 get_next_powerup_position() const;
    PowerupType get_next_powerup_type() const;
    const int get_powerup_id() const;
    const bool should_pickup_powerup(const int object_id, glm::vec3 object_pos) const;
    const bool should_update_powerup_position(const int object_id) const;

private:
    int powerup_id_;
    int frame_counter_;
    GameState game_state_;
    glm::vec3 powerup_pos_;
    PowerupType powerup_type_;
    std::map<int, PowerupType> object_powerups_;
};
