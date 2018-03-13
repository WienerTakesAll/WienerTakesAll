#pragma once

#include <map>

#include "EventSystem.h"
#include "GameState.h"
#include "PowerupSubsystem.h"
#include "ScoringSubsystem.h"

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
    void handle_new_it(const Event& e);
    void handle_object_transform_event(const Event& e);
    void handle_vehicle_collision(const Event& e);
    void handle_add_powerup(const Event& e);
    void handle_pickup_powerup(const Event& e);
    void handle_change_powerup(const Event& e);
    void handle_move_powerup(const Event& e);
    void handle_use_powerup(const Event& e);

    bool should_update_score() const;

    GameObjectCounter* gameobject_counter_;
    GameState current_game_state_;
    int current_it_id_;

    std::map<int, std::vector<float>> object_positions_;
    PowerupSubsystem powerup_subsystem_;
    ScoringSubsystem scoring_subsystem_;
};
