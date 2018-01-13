#pragma once

#include "PhysicsComponent.h"
#include "RenderingComponent.h"

class GameObject {
    int id;
    PhysicsComponent physics_component;
    RenderingComponent rendering_component;
    // Other params
};