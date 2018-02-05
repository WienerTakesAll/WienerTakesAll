#include <iostream>

#include "GameplaySystem.h"

#include "SDL.h"

GameplaySystem::GameplaySystem() {
    add_event_handler(EventType::LOAD_EVENT, &GameplaySystem::handle_load, this);
    add_event_handler(EventType::KEYPRESS_EVENT, &GameplaySystem::handle_key_press, this);
}

void GameplaySystem::update() {
    // Update game state here
}

void GameplaySystem::handle_load(const Event& e) {
    // Initialize gameplay here
    std::cout << "Gameplay initialized here" << std::endl;
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
