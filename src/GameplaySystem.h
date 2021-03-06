#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <map>

#include "EventSystem.h"
#include "GameState.h"
#include "PowerupSubsystem.h"
#include "ScoringSubsystem.h"
#include "StatusEffect.h"

class GameObjectCounter;

class GameplaySystem : public EventSystem<GameplaySystem> {
public:
    GameplaySystem();
    void update();

private:
    void handle_load(const Event& e);
    void handle_new_game_state(const Event& e);
    void handle_key_press(const Event& e);
    void handle_add_vehicle(const Event& e);
    void handle_add_charcoal(const Event& e);
    void handle_new_it(const Event& e);
    void handle_object_transform_event(const Event& e);
    void handle_vehicle_collision(const Event& e);
    void handle_add_powerup(const Event& e);
    void handle_pickup_powerup(const Event& e);
    void handle_change_powerup(const Event& e);
    void handle_use_powerup(const Event& e);
    void handle_new_status_effect(const Event& e);
    void handle_player_fell_off_arena(const Event& e);
    void handle_restore_controls(const Event& e);

    // Returns appropriate "opposite" key
    const int get_reverse_key(const int key) const;

    bool should_update_score() const;

    float calculate_player_speed(int player);

    GameObjectCounter* gameobject_counter_;
    GameState current_game_state_;
    int current_it_id_;

    std::map<int, glm::vec3> object_positions_;
    std::map<int, glm::quat> object_rotations_;
    std::map<int, glm::vec3> object_velocities_;

    struct PlayerStatusEffect {
        StatusEffect effect_;
        int duration_;
    };

    std::map<int, PlayerStatusEffect> player_status_effects_;
    std::array<bool, 4> controllers_reversed_;
    int dominator_;

    PowerupSubsystem powerup_subsystem_;
    ScoringSubsystem scoring_subsystem_;
};
