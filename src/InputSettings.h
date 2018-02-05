#pragma once
#include<map>

struct InputSettings {
    const int MAX_PLAYERS = 4; // Maximum number of players in a game
    int deadzone = 8000; // Minimum range of displacement for joystick before reading event
};
