#pragma once

#include "EventSystem.h"

class GameObjectCounter;

class GameplaySystem : public EventSystem<GameplaySystem> {
public:
    GameplaySystem();
    void update();

private:
    void handle_load(const Event& e);
    void handle_start_game(const Event& e);
    void handle_key_press(const Event& e);
    GameObjectCounter* gameobject_counter_;
};
