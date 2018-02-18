#include <iostream>

#include "SDL.h"

#include "GameObjectCounter.h"
#include "GameplaySystem.h"
#include "CarControlType.h"

GameplaySystem::GameplaySystem()
    : gameobject_counter_(GameObjectCounter::get_instance()) {
    add_event_handler(EventType::LOAD_EVENT, &GameplaySystem::handle_load, this);
    add_event_handler(EventType::KEYPRESS_EVENT, &GameplaySystem::handle_key_press, this);
}

void GameplaySystem::update() {
    // Update game state here
}

void GameplaySystem::handle_load(const Event& e) {
    EventSystem::queue_event(
        Event(
            EventType::ADD_CAR,
            "object_id", gameobject_counter_->assign_id(),
            // TODO: Pass glm::vec3 in events
            "pos_x", 4,
            "pos_y", 2,
            "pos_z", 0//,
            // "name", "Car 1"
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
    int key = e.get_value<int>("key", true).first;
    int value = e.get_value<int>("value", true).first;

    std::vector<Event> new_events;

    switch (key) {
        case SDLK_w:
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::FORWARD_DRIVE,
                                    "value", 0.5f);
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::BRAKE,
                                    "value", 0.0f);
            break;

        case SDLK_s:
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::FORWARD_DRIVE,
                                    "value", 0.0f);
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::BRAKE,
                                    "value", 0.5f);
            break;

        case SDLK_a:
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::STEER,
                                    "value", 0.5f);
            break;

        case SDLK_d:
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::STEER,
                                    "value", -0.5f);
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::FORWARD_DRIVE,
                                    "value", (float)value / 32768);
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::BRAKE,
                                    "value", (float)value / 32768);
            break;

        case SDL_CONTROLLER_AXIS_LEFTX:
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::STEER,
                                    "value", (float)value / -32768);
            break;

        case SDL_CONTROLLER_BUTTON_B:
            new_events.emplace_back(EventType::CAR_CONTROL,
                                    "type", CarControlType::HAND_BRAKE,
                                    "value", true);
            break;

        default:
            return;
    }

    for (const auto& event : new_events) {
        EventSystem::queue_event(Event(event));
    }
}
