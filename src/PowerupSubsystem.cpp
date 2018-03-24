#include <stdlib.h> /* srand, rand */
#include <time.h> /* time */
#include <algorithm>
#include <vector>

#include "SDL.h"
#include <glm/gtx/vector_angle.hpp>

#include "PowerupSubsystem.h"

namespace {
    const float POWERUP_DISTANCE_THRESHOLD = 2.5f;
    const int POWERUP_LOCK_FRAMES = 120;
    const glm::vec3 POWERUP_LOCATION_LIMITS = glm::vec3(10.0f, 1.5f, 10.0f);
    const float KETCHUP_BOOST = 50000.0f;
    const glm::vec3 HOT_KNOCK_BACK_FORCE(15000.f, 80000.f, 15000.f);
}

PowerupSubsystem::PowerupSubsystem()
    : powerup_id_(-1)
    , frame_counter_(0)
    , powerup_type_(PowerupType::POWERUP_COUNT) {
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

void PowerupSubsystem::change_powerup_position(const int object_id, glm::vec3 pos) {
    powerup_pos_ = pos;
}

void PowerupSubsystem::change_powerup_type(const PowerupType new_type) {
    powerup_type_ = new_type;
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

std::vector<Event>&& PowerupSubsystem::use_powerup(
    const int object_id,
    const std::map<int, glm::quat>& object_rotations
) {
    // Check if object_id has a pre-existing powerup. If so, return that. Else, return NONE.
    PowerupType type = object_powerups_.find(object_id) == object_powerups_.end()
                       ? PowerupType::POWERUP_COUNT
                       : object_powerups_[object_id];

    // Clear powerup entry.
    object_powerups_[object_id] = PowerupType::POWERUP_COUNT;

    std::vector<Event> powerup_events;

    switch (type) {
        case PowerupType::KETCHUP: {
            std::cout << "KETCHUP used by player " << object_id << std::endl;
            glm::vec3 boost_direction = object_rotations.at(object_id) * glm::vec3(0.0f, 0.0f, KETCHUP_BOOST);
            powerup_events.emplace_back(
                EventType::OBJECT_APPLY_FORCE,
                "object_id", object_id,
                "x", boost_direction.x,
                "y", boost_direction.y,
                "z", boost_direction.z
            );
            break;
        }

        case PowerupType::PICKLE:
            std::cout << "PICKLE used by player " << object_id << std::endl;
            break;

        case PowerupType::HOT:
            std::cout << "HOT used by player " << object_id << std::endl;
            for (int i = 0; i < 4; ++i) {
                if (i == object_id) {
                    continue;
                }

                powerup_events.emplace_back(
                    EventType::OBJECT_APPLY_FORCE,
                    "object_id", i,
                    // TODO: Pass glm::vec3 in events
                    "x", HOT_KNOCK_BACK_FORCE.x,
                    "y", HOT_KNOCK_BACK_FORCE.y,
                    "z", HOT_KNOCK_BACK_FORCE.z
                );
            }

            break;

        default:
            break;
    }

    return std::move(powerup_events);
}

const bool PowerupSubsystem::can_use_powerup(const int object_id) const {
    return
        object_powerups_.find(object_id) != object_powerups_.end() &&
        object_powerups_.at(object_id) != PowerupType::POWERUP_COUNT;
}

const bool PowerupSubsystem::is_powerup(const int object_id) const {
    return powerup_id_ == object_id;
}

glm::vec3 PowerupSubsystem::get_next_powerup_position() const {
    float x = (rand() % (2 * (int) POWERUP_LOCATION_LIMITS.x)) - (int) POWERUP_LOCATION_LIMITS.x;
    float y = 1.5f;
    float z = (rand() % (2 * (int) POWERUP_LOCATION_LIMITS.z)) - (int) POWERUP_LOCATION_LIMITS.z;
    return glm::vec3(x, y, z);
}

PowerupType PowerupSubsystem::get_next_powerup_type() const {
    // Random powerup type
    PowerupType new_type = static_cast<PowerupType>(rand() % (int) PowerupType::POWERUP_COUNT);
    return new_type;
}

const int PowerupSubsystem::get_powerup_id() const {
    return powerup_id_;
}

const bool PowerupSubsystem::should_pickup_powerup(const int object_id, glm::vec3 object_pos) const {
    return
        !is_powerup(object_id) &&
        frame_counter_ >= POWERUP_LOCK_FRAMES &&
        glm::distance(powerup_pos_, object_pos) <= POWERUP_DISTANCE_THRESHOLD;
}

const bool PowerupSubsystem::should_update_powerup_position(const int object_id) const {
    return frame_counter_ >= POWERUP_LOCK_FRAMES;
}
