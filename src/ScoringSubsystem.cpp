#include <assert.h>

#include "ScoringSubsystem.h"

namespace {
    const int SCORE_LOCK_FRAMES_START = 300;
}

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
    // Update score of current it
    if (score_lock_frames_ > 0) {
        --score_lock_frames_;
    } else {
        ++scores_[current_it_id_];
    }
}

void ScoringSubsystem::set_new_game_state(const GameState new_game_state) {
    if (new_game_state == GameState::START_MENU) {
        scores_.clear();
    } else if (new_game_state == GameState::IN_GAME) {
        score_lock_frames_ = SCORE_LOCK_FRAMES_START;
    }

    game_state_ = new_game_state;
}

void ScoringSubsystem::set_new_it_id(const int new_it_id) {
    current_it_id_ = new_it_id;
}

int ScoringSubsystem::get_current_it_score() {
    return scores_[current_it_id_];
}

int ScoringSubsystem::get_player_score(int player) {
    auto score = scores_.find(player);

    if (score == scores_.end()) {
        // Avoid crash
        return 0;
    }

    return score->second;
}

int ScoringSubsystem::get_current_score_lock_frames() {
    return score_lock_frames_;
}
