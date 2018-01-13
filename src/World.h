#pragma once

#include <unordered_map>
#include "GameObject.h"

#include "SDL.h"

class World {
    std::unordered_map<int, GameObject> game_objects;
};