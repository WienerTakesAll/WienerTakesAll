#pragma once

#include <map>

#include "EventSystem.h"
#include "GameState.h"

class GameObjectCounter;

class GameplaySystem : public EventSystem<GameplaySystem> {
public:
    GameplaySystem();
    void update();

private:
    void handle_load(const Event& e);
    void handle_new_game_state(const Event& e);
    void handle_key_press(const Event& e);
    void handle_object_transform_event(const Event& e);
    void handle_vehicle_collision(const Event& e);

    std::map<int, std::vector<float>> object_positions_;
    GameObjectCounter* gameobject_counter_;
    GameState current_game_state_;
};
