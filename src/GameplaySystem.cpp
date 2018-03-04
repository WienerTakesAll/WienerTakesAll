#include <iostream>

#include "SDL.h"

#include "GameObjectCounter.h"
#include "GameplaySystem.h"
#include "VehicleControls.h"

GameplaySystem::GameplaySystem()
    : gameobject_counter_(GameObjectCounter::get_instance())
    , current_game_state_(GameState::START_MENU)
    , current_it_id_(-1) {
    add_event_handler(EventType::LOAD_EVENT, &GameplaySystem::handle_load, this);
    add_event_handler(EventType::KEYPRESS_EVENT, &GameplaySystem::handle_key_press, this);
    add_event_handler(EventType::NEW_GAME_STATE, &GameplaySystem::handle_new_game_state, this);
    add_event_handler(EventType::ADD_VEHICLE, &GameplaySystem::handle_add_vehicle, this);
    add_event_handler(EventType::OBJECT_TRANSFORM_EVENT, &GameplaySystem::handle_object_transform_event, this);
    add_event_handler(EventType::VEHICLE_COLLISION, &GameplaySystem::handle_vehicle_collision, this);

    EventSystem::queue_event(
        Event(
            EventType::NEW_GAME_STATE,
            "state", GameState::START_MENU
        )
    );
}

void GameplaySystem::update() {
    // Update game state here
    scoring_subsystem_.update();
}

void GameplaySystem::handle_load(const Event& e) {

}

void GameplaySystem::handle_new_game_state(const Event& e) {
    GameState new_game_state = (GameState)e.get_value<int>("state", true).first;

    scoring_subsystem_.set_new_game_state(new_game_state);

    if (new_game_state == GameState::IN_GAME) {
        int num_humans = e.get_value<int>("num_players", true).first;
        std::cout << "starting game with " << num_humans << " human players" << std::endl;
        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", gameobject_counter_->assign_id(),
                // TODO: Pass glm::vec3 in events
                "pos_x", 4,
                "pos_y", 2,
                "pos_z", 0//,
                // "name", "Vehicle 1"
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", gameobject_counter_->assign_id(),
                // TODO: Pass glm::vec3 in events
                "pos_x", 10,
                "pos_y", 2,
                "pos_z", 0//,
                // "name", "Vehicle 1"
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", gameobject_counter_->assign_id(),
                // TODO: Pass glm::vec3 in events
                "pos_x", -4,
                "pos_y", 2,
                "pos_z", 0//,
                // "name", "Vehicle 1"
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", gameobject_counter_->assign_id(),
                // TODO: Pass glm::vec3 in events
                "pos_x", -10,
                "pos_y", 2,
                "pos_z", 0//,
                // "name", "Vehicle 1"
            )
        );

        // Terrain
        EventSystem::queue_event(
            Event(
                EventType::ADD_ARENA,
                "object_id", gameobject_counter_->assign_id()
            )
        );

        // Terrain
        EventSystem::queue_event(
            Event(
                EventType::ACTIVATE_AI,
                "num_ai", 4 - num_humans
            )
        );
    }

    current_game_state_ = new_game_state;
}

void GameplaySystem::handle_key_press(const Event& e) {
    if (current_game_state_ != GameState::IN_GAME) {
        return;
    }

    // Update gameplay during keypress
    int key = e.get_value<int>("key", true).first;
    int player_id = e.get_value<int>("player_id", true).first;
    int value = e.get_value<int>("value", true).first;

    std::vector<Event> new_events;

    switch (key) {
        // Keyboard acceleration
        case SDLK_w: // fall through
        case SDLK_t:
        case SDLK_i:
        case SDLK_UP: {
            if (value == SDL_KEYDOWN) {
                new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                        "index", player_id,
                                        "type", VehicleControlType::FORWARD_DRIVE,
                                        "value", 0.5f);
                new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                        "index", player_id,
                                        "type", VehicleControlType::BRAKE,
                                        "value", 0.0f);
            } else if (value == SDL_KEYUP) {
                new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                        "index", player_id,
                                        "type", VehicleControlType::FORWARD_DRIVE,
                                        "value", 0.0f);
            }

            break;
        }

        // keyboard braking
        case SDLK_s: // fall through
        case SDLK_g:
        case SDLK_k:
        case SDLK_DOWN: {
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::FORWARD_DRIVE,
                                    "value", 0.0f);
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::BRAKE,
                                    "value", 0.5f);
            break;
        }

        // keyboard left steer
        case SDLK_a: // fall through
        case SDLK_f:
        case SDLK_j:
        case SDLK_LEFT: {
            float steer_amount_left = 0.5f;

            if (value == SDL_KEYUP) {
                steer_amount_left = 0.0f;
            }

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", steer_amount_left);
            break;
        }

        // keyboard right steer
        case SDLK_d: // fall through
        case SDLK_h:
        case SDLK_l:
        case SDLK_RIGHT: {
            float steer_amount_right = -0.5f;

            if (value == SDL_KEYUP) {
                steer_amount_right = 0.0f;
            }

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", steer_amount_right);
            break;
        }

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::FORWARD_DRIVE,
                                    "value", (float)value / 32768);
            break;
        }

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::BRAKE,
                                    "value", (float)value / 32768);
            break;
        }

        case SDL_CONTROLLER_AXIS_LEFTX: {
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", (float)value / -32768);
            break;
        }

        case SDL_CONTROLLER_BUTTON_B: {
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::HAND_BRAKE,
                                    "value", true);
            break;
        }

        default:
            return;
    }

    for (const auto& event : new_events) {
        EventSystem::queue_event(Event(event));
    }
}

void GameplaySystem::handle_add_vehicle(const Event& e) {
    std::pair<int, bool> object_id = e.get_value<int>("object_id", true);
    scoring_subsystem_.add_vehicle(object_id.first);

    // Temporary, set first vehicle to be added as first it
    if (current_it_id_ == -1) {
        current_it_id_ = object_id.first;
        scoring_subsystem_.set_new_it_id(current_it_id_);
    }
}
void GameplaySystem::handle_object_transform_event(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    float x = e.get_value<float>("pos_x", true).first;
    float y = e.get_value<float>("pos_y", true).first;
    float z = e.get_value<float>("pos_z", true).first;

    object_locations_[object_id] = {x, y, z};
}

void GameplaySystem::handle_vehicle_collision(const Event& e) {
    int a_id = e.get_value<int>("a_id", true).first;
    int b_id = e.get_value<int>("b_id", true).first;
    std::cout << a_id << " collided with " << b_id << std::endl;

    if (current_it_id_ == a_id) {
        current_it_id_ = b_id;
        scoring_subsystem_.set_new_it_id(current_it_id_);
    } else if (current_it_id_ == b_id) {
        current_it_id_ = a_id;
        scoring_subsystem_.set_new_it_id(current_it_id_);
    }
}
