#pragma once
#include "SettingType.h"
#include<map>

namespace InputSettings {
    const int MAX_PLAYERS = 4; // Maximum number of players in a game
    extern int DEADZONE; // Minimum range of displacement for joystick before reading event

    const std::map<std::string, std::pair<const SettingType, void* const>> CONFIG_MAPPING = {
        {"deadzone", {SettingType::INT, &DEADZONE}}
    };
}
