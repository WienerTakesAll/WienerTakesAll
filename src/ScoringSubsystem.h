#pragma once

#include <map>

#include "EventSystem.h"
#include "GameState.h"

class ScoringSubsystem {

public:
    ScoringSubsystem();
    void add_vehicle(const int object_id);
    void update();
    void handle_new_game_state(const GameState new_game_state);

private:
    GameState game_state_;
    int current_it_;
    std::map<int, unsigned int> scores_;
};
