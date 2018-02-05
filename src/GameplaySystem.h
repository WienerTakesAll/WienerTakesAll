#pragma once

#include "EventSystem.h"

class GameplaySystem : public EventSystem<GameplaySystem> {
public:
    GameplaySystem();
    void update();

private:
    void handle_load(const Event& e);
    void handle_key_press(const Event& e);
};
