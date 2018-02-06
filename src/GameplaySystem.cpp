#include <iostream>

#include "GameObjectCounter.h"
#include "GameplaySystem.h"

#include "SDL.h"

GameplaySystem::GameplaySystem() {
    add_event_handler(EventType::LOAD_EVENT, &GameplaySystem::handle_load, this);
    add_event_handler(EventType::KEYPRESS_EVENT, &GameplaySystem::handle_key_press, this);
}

void GameplaySystem::update() {
    // Update game state here
    EventSystem::queue_event (
        Event (
            EventType::EXAMPLE_SHIP_IDLE_EVENT,
            "object_id", 1,
            "rotation_rad", 0.01f
        )
    );
}

void GameplaySystem::handle_load(const Event& e) {
    // Initialize gameplay here
    std::cout << "Gameplay initialized here" << std::endl;

    GameObjectCounter* counter = GameObjectCounter::get_instance();

    // Ship 1
    EventSystem::queue_event(
        Event(
            EventType::ADD_EXAMPLE_SHIP_EVENT,
            "object_id", counter->assign_id(),
            // TODO: Pass glm::vec3 in events
            "pos_x", 0,
            "pos_y", -2,
            "pos_z", 0//,
            // "name", "Ship 1"
        )
    );

    // Ship 2
    EventSystem::queue_event(
        Event(
            EventType::ADD_EXAMPLE_SHIP_EVENT,
            "object_id", counter->assign_id(),
            // TODO: Pass glm::vec3 in events
            "pos_x", 1,
            "pos_y", 2,
            "pos_z", 1//,
            // "name", "Ship 2"
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
    std::cout << "[Test] Gameplay handled keypress" << std::endl; // Remove when function is implemented
}
