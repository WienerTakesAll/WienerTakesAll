#pragma once
#include<map>

struct InputSettings {
    int max_players = 4; // Maximum number of players in a game
    int deadzone = 8000; // Minimum range of displacement for joystick before reading event
};
