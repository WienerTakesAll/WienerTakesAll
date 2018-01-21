#include "GameObject.h"

unsigned int GameObject::game_object_count = 0;

GameObject::GameObject()
    : id_(game_object_count)
    , physics_component_(PhysicsComponent())
    , rendering_component_(RenderingComponent()) {
    ++game_object_count;
}

unsigned int GameObject::id() const {
    return id_;
}

PhysicsComponent GameObject::physics_component() const {
    return physics_component_;
}

RenderingComponent GameObject::rendering_component() const {
    return rendering_component_;
}
