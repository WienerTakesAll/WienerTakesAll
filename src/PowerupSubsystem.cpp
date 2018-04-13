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

    const int POWERUP_INDEX_RANGE = PowerupType::NO_POWERUP;
    const int NUM_PLAYERS = 4;
}

PowerupSubsystem::PowerupSubsystem() {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        player_powerups_[i] = PowerupType::NO_POWERUP;
    }
}
void PowerupSubsystem::load() {
    // Initialize random seed
    srand(time(NULL));
}

void PowerupSubsystem::update() {
    for (auto& powerup_obj : powerup_objs_) {
        powerup_obj.second.frame_lock_counter_ =
            std::min(POWERUP_LOCK_FRAMES, powerup_obj.second.frame_lock_counter_ + 1);
    }
}

void PowerupSubsystem::add_mound_location(const int x, int y, const int z) {
    charcoal_locations.emplace_back(glm::vec3(x, y + 3, z));
}

void PowerupSubsystem::set_new_game_state(const GameState new_game_state) {
    if (new_game_state == GameState::IN_GAME) {
        for (auto& player : player_powerups_) {
            player.second = PowerupType::NO_POWERUP;
        }

        charcoal_locations.clear();
    }

    game_state_ = new_game_state;
}

void PowerupSubsystem::create_powerup(const int object_id, const PowerupType type, glm::vec3 pos) {
    PowerupObject new_powerup;
    new_powerup.pos_ = pos;
    new_powerup.type_ = type;
    new_powerup.frame_lock_counter_ = 0;
    powerup_objs_[object_id] = new_powerup;
}

void PowerupSubsystem::change_powerup_type(int object_id, const PowerupType new_type) {
    powerup_objs_[object_id].type_ = new_type;
}

// Picksup a powerup for a player
// Stores the new powerup in the inventory of player_id
// Updates the position of the powerup
// Returns the new location of the powerup
glm::vec3 PowerupSubsystem::pickup_powerup(const int powerup_id, const int player_id) {
    // Add current powerup to object
    player_powerups_[player_id] = powerup_objs_[powerup_id].type_;

    glm::vec3 new_powerup_pos = get_next_powerup_position();
    powerup_objs_[powerup_id].pos_ = new_powerup_pos;
    powerup_objs_[powerup_id].frame_lock_counter_ = 0;
    return new_powerup_pos;
}

PowerupType PowerupSubsystem::spend_powerup(const int object_id) {
    PowerupType spent_powerup = player_powerups_[object_id];
    player_powerups_[object_id] = PowerupType::NO_POWERUP;
    return spent_powerup;
}

const bool PowerupSubsystem::is_powerup(const int object_id) const {
    for (const auto& powerup_obj : powerup_objs_) {
        if (powerup_obj.first == object_id) {
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

PowerupType PowerupSubsystem::get_powerup_type(int powerup_id) const {
    return powerup_objs_.at(powerup_id).type_;
}

// Returns a vector of all powerup ids within pickup range
std::vector<int> PowerupSubsystem::within_powerup(glm::vec3 object_pos) const {
    std::vector<int> in_range_powerups;

    for (const auto& powerup_obj : powerup_objs_) {
        if (powerup_obj.second.frame_lock_counter_ >= POWERUP_LOCK_FRAMES &&
                glm::distance(powerup_obj.second.pos_, object_pos) <= POWERUP_DISTANCE_THRESHOLD) {

            in_range_powerups.push_back(powerup_obj.first);
        }
    }

    return in_range_powerups;
}
