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
    int get_current_it_score();
    int get_player_score(int player);
    int get_current_lock_frames();

private:
    int current_it_id_;
    GameState game_state_;
    std::map<int, int> scores_;
    int lock_frames_;
};
