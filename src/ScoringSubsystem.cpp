#include "ScoringSubsystem.h"

#include <iostream>

ScoringSubsystem::ScoringSubsystem()
    : current_it_id_(-1) {
}

void ScoringSubsystem::add_vehicle(const int object_id) {
    // Early exit if vehicle is already added
    if (scores_.find(object_id) != scores_.end()) {
        return;
    }

    scores_[object_id] = 0;
}

void ScoringSubsystem::update() {
    if (game_state_     != GameState::IN_GAME ||
            scores_.size()  == 0                  ||
            current_it_id_  == -1 ) {
        return;
    }

    // Update score of current it
    ++scores_[current_it_id_];

    if (scores_[current_it_id_] % 64 == 0) {
        std::cout << "Player[" << current_it_id_ << "] = " << scores_[current_it_id_] << std::endl;
    }

}

void ScoringSubsystem::set_new_game_state(const GameState new_game_state) {
    game_state_ = new_game_state;
}

void ScoringSubsystem::set_new_it_id(const int new_it_id) {
    current_it_id_ = new_it_id;
}
