#include <stdlib.h> /* srand, rand */
#include <time.h> /* time */

#include "SDL.h"
#include <glm/gtx/vector_angle.hpp>

#include "PowerupSubsystem.h"

namespace {
    const float POWERUP_DISTANCE_THRESHOLD = 2.5f;
    const int POWERUP_LOCK_FRAMES = 120;
}

PowerupSubsystem::PowerupSubsystem()
    : powerup_id_(-1)
    , frame_counter_(0)
    , powerup_type_(PowerupType::NONE) {
}

void PowerupSubsystem::load() {
    // Initialize random seed
    srand(time(NULL));
}

void PowerupSubsystem::update() {
    frame_counter_ = std::min(POWERUP_LOCK_FRAMES, frame_counter_ + 1);
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

const bool PowerupSubsystem::should_pickup_powerup(const int object_id, glm::vec3 object_pos) const {
    return
        !is_powerup(object_id) &&
        frame_counter_ >= POWERUP_LOCK_FRAMES &&
        glm::distance(powerup_pos_, object_pos) <= POWERUP_DISTANCE_THRESHOLD;
}

void PowerupSubsystem::pickup_powerup(const int object_id) {
    // Prevent pickup if powerup was just picked up
    if (frame_counter_ < POWERUP_LOCK_FRAMES) {
        return;
    }

    // Add current powerup to object
    object_powerups_[object_id] = powerup_type_;

    // Lock powerup until new powerup comes
    frame_counter_ = 0;
}

void PowerupSubsystem::change_powerup_type(const PowerupType new_type) {
    powerup_type_ = new_type;
}

void PowerupSubsystem::move_powerup(const int object_id, glm::vec3 pos) {
    powerup_pos_ += pos;
}

const int PowerupSubsystem::get_powerup_id() const {
    return powerup_id_;
}

PowerupType PowerupSubsystem::get_next_powerup_type() const {
    // Random powerup type
    PowerupType new_type = static_cast<PowerupType>(rand() % (int) PowerupType::NONE);
    return new_type;
}

const bool PowerupSubsystem::is_powerup(const int object_id) const {
    return powerup_id_ == object_id;
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
