#pragma once

#include "EventSystem.h"
#include "GameState.h"
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
    GameObjectCounter* gameobject_counter_;
    GameState current_game_state_;
    ScoringSubsystem scoring_subsystem_;
};
