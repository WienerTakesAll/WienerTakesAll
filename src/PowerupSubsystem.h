#pragma once

#include <glm/glm.hpp>
#include <map>

#include "GameState.h"
#include "Powerup.h"
#include "Event.h"

class PowerupSubsystem {
public:
    PowerupSubsystem();
    void load();
    void update();
    void set_new_game_state(const GameState new_game_state);
    void add_mound_location(const int x, int y, const int z);
    void create_powerup(const int object_id, const PowerupType type, glm::vec3 pos);
    // Modifiers
    void change_powerup_type(int object_id, const PowerupType new_type);
    glm::vec3 pickup_powerup(const int powerup_id, const int player_id);
    PowerupType spend_powerup(const int object_id);

    // Utility functions
    const bool is_powerup(const int object_id) const;
    PowerupType get_next_powerup_type() const;
    PowerupType get_powerup_type(int powerup_id) const;
    std::vector<int> within_powerup(glm::vec3 object_pos) const;

private:
    glm::vec3 get_next_powerup_position() const;

    GameState game_state_;
    struct PowerupObject {
        glm::vec3 pos_;
        PowerupType type_;
        int frame_lock_counter_;
    };
    std::map<int, PowerupObject> powerup_objs_;
    std::map<int, PowerupType> player_powerups_;
    std::vector<glm::vec3> charcoal_locations;
};
