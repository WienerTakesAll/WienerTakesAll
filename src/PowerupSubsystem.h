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

    const bool should_pickup_powerup(const int object_id, glm::vec3 object_pos) const;
    void pickup_powerup(const int object_id);
    void change_powerup_type(const PowerupType new_type);
    void move_powerup(const int object_id, glm::vec3 pos);

    const int get_powerup_id() const;
    PowerupType get_next_powerup_type() const;
    const bool is_powerup(const int object_id) const;
    const bool can_use_powerup(const int object_id) const;

    PowerupType use_powerup(const int object_id);

private:
    int powerup_id_;
    int frame_counter_;
    GameState game_state_;
    glm::vec3 powerup_pos_;
    PowerupType powerup_type_;
    std::map<int, PowerupType> object_powerups_;
};
