#pragma once

#include <map>

#include "GameState.h"
#include "Powerup.h"

class PowerupSubsystem {
public:
    PowerupSubsystem();
    void load();
    void set_new_game_state(const GameState new_game_state);

    void create_powerup(const int object_id, const PowerupType type);
    PowerupType get_next_powerup_type() const;

    PowerupType use_powerup(const int object_id);
    void pickup_powerup(const int object_id);

private:
    int powerup_id_;
    GameState game_state_;
    PowerupType powerup_type_;
    std::map<int, PowerupType> object_powerups_;
};
