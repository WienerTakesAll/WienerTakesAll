#include <stdlib.h> /* srand, rand */
#include <time.h> /* time */

#include "SDL.h"
#include <glm/gtx/vector_angle.hpp>

#include "PowerupSubsystem.h"

PowerupSubsystem::PowerupSubsystem()
    : powerup_id_(-1)
    , powerup_type_(PowerupType::NONE) {
}

void PowerupSubsystem::load() {
    // Initialize random seed
    srand(time(NULL));
}

void PowerupSubsystem::set_new_game_state(const GameState new_game_state) {
    if (new_game_state == GameState::IN_GAME) {
        object_powerups_.clear();
    }

    game_state_ = new_game_state;
}

void PowerupSubsystem::create_powerup(const int object_id, const PowerupType type, glm::vec3 pos) {
    powerup_id_ = object_id;
    powerup_type_ = type;
    powerup_pos_ = pos;
}

PowerupType PowerupSubsystem::get_next_powerup_type() const {
    // Random powerup type
    PowerupType new_type = static_cast<PowerupType>(rand() % (int) PowerupType::NONE);
    return new_type;
}

PowerupType PowerupSubsystem::use_powerup(const int object_id) {
    // Check if object_id has a pre-existing powerup. If so, return that. Else, return NONE.
    PowerupType type = object_powerups_.find(object_id) == object_powerups_.end()
                       ? PowerupType::NONE
                       : object_powerups_[object_id];

    // Clear powerup entry.
    object_powerups_[object_id] = PowerupType::NONE;

    return type;
}

void PowerupSubsystem::pickup_powerup(const int object_id) {
    object_powerups_[object_id] = powerup_type_;
}
