#include "ScoringSubsystem.h"

#include <iostream>

ScoringSubsystem::ScoringSubsystem()
    : current_it_(-1) {
}

void ScoringSubsystem::add_vehicle(const int object_id) {
    // Early exit if vehicle is already added
    if (scores_.find(object_id) != scores_.end()) {
        return;
    }

    // First vehicle to be added is it first
    if (scores_.size() == 0) {
        current_it_ = object_id;
    }

    scores_[object_id] = 0;
}

void ScoringSubsystem::update() {
    if (game_state_ != GameState::IN_GAME) {
        return;
    }

    if (scores_.size() == 0) {
        return;
    }

    if (current_it_ == -1) {
        return;
    }

    // Update score of current it
    ++scores_[current_it_];

    std::cout << "Player[" << current_it_ << "] = " << scores_[current_it_] << std::endl;
}

void ScoringSubsystem::handle_new_game_state(const GameState new_game_state) {
    game_state_ = new_game_state;
}
