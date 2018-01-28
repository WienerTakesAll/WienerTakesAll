#pragma once

#include "PhysicsComponent.h"
#include "RenderingComponent.h"

class GameObject {
public:
    GameObject();
    unsigned int id() const;
    PhysicsComponent physics_component() const;
    RenderingComponent rendering_component() const;

protected:
    unsigned int id_;
    PhysicsComponent physics_component_;
    RenderingComponent rendering_component_;

private:
    static unsigned int game_object_count;
};
