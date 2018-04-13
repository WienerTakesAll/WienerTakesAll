#include <iostream>
#include <stdlib.h> /* srand, rand */
#include <time.h> /* time */
#include <algorithm>
#include <vector>

#include "SDL.h"
#include <glm/gtx/vector_angle.hpp>

#include "PowerupSubsystem.h"

namespace {
    const float POWERUP_DISTANCE_THRESHOLD = 2.5f;
    const int POWERUP_LOCK_FRAMES = 30;

    const int POWERUP_INDEX_RANGE = PowerupType::POWERUP_COUNT;
}

PowerupSubsystem::PowerupSubsystem()
    : frame_counter_(0) {
}
void PowerupSubsystem::load() {
    // Initialize random seed
    srand(time(NULL));
}

void PowerupSubsystem::update() {
    frame_counter_ = std::min(POWERUP_LOCK_FRAMES, frame_counter_ + 1);
}

void PowerupSubsystem::add_mound_location(const int x, int y, const int z) {
    charcoal_locations.emplace_back(glm::vec3(x, y + 3, z));
}

void PowerupSubsystem::set_new_game_state(const GameState new_game_state) {
    if (new_game_state == GameState::IN_GAME) {
        player_powerups_.clear();
        charcoal_locations.clear();
    }

    game_state_ = new_game_state;
}

void PowerupSubsystem::create_powerup(const int object_id, const PowerupType type, glm::vec3 pos) {
    PowerupObject new_powerup;
    new_powerup.id_ = object_id;
    new_powerup.pos_ = pos;
    new_powerup.type_ = type;
    powerup_objs_.push_back(new_powerup);
}

void PowerupSubsystem::change_powerup_position(const int object_id, glm::vec3 pos) {
    powerup_objs_[0].pos_ = pos;
}

void PowerupSubsystem::change_powerup_type(int powerup_id, const PowerupType new_type) {
    powerup_objs_[0].type_ = new_type;
}

void PowerupSubsystem::pickup_powerup(const int object_id) {
    // Prevent pickup if powerup was just picked up
    if (frame_counter_ < POWERUP_LOCK_FRAMES) {
        return;
    }

    // Add current powerup to object
    player_powerups_[object_id] = powerup_objs_[0].type_;

    // Lock powerup until new powerup comes
    frame_counter_ = 0;
}

void PowerupSubsystem::spend_powerup(const int object_id) {
    // Clear powerup entry.
    player_powerups_[object_id] = PowerupType::POWERUP_COUNT;
}

const bool PowerupSubsystem::can_use_powerup(const int object_id) const {
    return
        player_powerups_.find(object_id) != player_powerups_.end() &&
        player_powerups_.at(object_id) != PowerupType::POWERUP_COUNT;
}

const PowerupType PowerupSubsystem::get_player_powerup_type(const int object_id) const {
    assert(can_use_powerup(object_id));
    return player_powerups_.at(object_id);
}

const bool PowerupSubsystem::is_powerup(const int object_id) const {
    for (const auto& powerup_obj : powerup_objs_) {
        if (powerup_obj.id_ == object_id) {
            return true;
        }
    }

    return false;
}

glm::vec3 PowerupSubsystem::get_next_powerup_position() const {
    int i = rand() % charcoal_locations.size();

    glm::vec3 location = charcoal_locations.at(i);

    return location;
}

PowerupType PowerupSubsystem::get_next_powerup_type() const {
    // Random powerup type
    PowerupType new_type = static_cast<PowerupType>(rand() % POWERUP_INDEX_RANGE);
    return new_type;
}

PowerupType PowerupSubsystem::get_powerup_type() const {
    return powerup_objs_[0].type_;
}

const int PowerupSubsystem::get_powerup_id() const {
    if (powerup_objs_.size() == 0) {
        return -1;
    }

    return powerup_objs_[0].id_;
}

const bool PowerupSubsystem::should_pickup_powerup(const int object_id, glm::vec3 object_pos) const {
    return
        !is_powerup(object_id) &&
        frame_counter_ >= POWERUP_LOCK_FRAMES &&
        glm::distance(powerup_objs_[0].pos_, object_pos) <= POWERUP_DISTANCE_THRESHOLD;
}

const bool PowerupSubsystem::should_update_powerup_position(const int object_id, const glm::vec3& position) const {
    return frame_counter_ >= POWERUP_LOCK_FRAMES && powerup_objs_[0].pos_ != position;
}
