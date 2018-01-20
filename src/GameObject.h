#pragma once

#include "PhysicsComponent.h"
#include "RenderingComponent.h"

class GameObject {
public:
    int id_;
    PhysicsComponent physics_component_;
    RenderingComponent rendering_component_;
};