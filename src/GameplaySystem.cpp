#include <iostream>

#include "GameObjectCounter.h"
#include "GameplaySystem.h"

#include "SDL.h"

GameplaySystem::GameplaySystem()
: gameobject_counter_(GameObjectCounter::get_instance()) {
    add_event_handler(EventType::LOAD_EVENT, &GameplaySystem::handle_load, this);
    add_event_handler(EventType::KEYPRESS_EVENT, &GameplaySystem::handle_key_press, this);
}

void GameplaySystem::update() {
    // Update game state here
}

void GameplaySystem::handle_load(const Event& e) {
    // Ship 1
    EventSystem::queue_event(
        Event(
            EventType::ADD_EXAMPLE_SHIP_EVENT,
            "object_id", gameobject_counter_->assign_id(),
            // TODO: Pass glm::vec3 in events
            "pos_x", 4,
            "pos_y", 2,
            "pos_z", 0//,
            // "name", "Ship 1"
        )
    );

    // Terrain
    EventSystem::queue_event(
        Event(
            EventType::ADD_TERRAIN_EVENT,
            "object_id", gameobject_counter_->assign_id()
        )
    );

}

void GameplaySystem::handle_key_press(const Event& e) {
    // Update gameplay during keypress
    /* UNCOMMENT WHEN READY FOR USE
    int player_id = e.get_value<int>("player_id", -1);
    int key = e.get_value<int>("key", -1);
    int value = e.get_value<int>("value", 0);

    switch (key) {
        case SDLK_LEFT:
            break;

        case SDLK_RIGHT:
            break;

        case SDLK_UP:
            break;

        case SDLK_DOWN:
            break;

        default:
            break;
    }
    */
    // std::cout << "[Test] Gameplay handled keypress" << std::endl; // Remove when function is implemented
}
