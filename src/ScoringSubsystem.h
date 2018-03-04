#pragma once

#include <map>

#include "EventSystem.h"
#include "GameState.h"

class ScoringSubsystem {

public:
    ScoringSubsystem();
    void add_vehicle(const int object_id);
    void update();
    void set_new_game_state(const GameState new_game_state);
    void set_new_it_id(const int new_it_id);

private:
    int current_it_id_;
    GameState game_state_;
    std::map<int, unsigned int> scores_;
};
