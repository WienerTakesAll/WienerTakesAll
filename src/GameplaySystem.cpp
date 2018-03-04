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
    add_event_handler(EventType::NEW_IT, &GameplaySystem::handle_new_it, this);

    EventSystem::queue_event(
        Event(
            EventType::NEW_GAME_STATE,
            "state", GameState::START_MENU
        )
    );
}

void GameplaySystem::update() {
    // Update game state here
    if (should_update_score()) {
        scoring_subsystem_.update();

        EventSystem::queue_event(
            Event(
                EventType::UPDATE_SCORE,
                "object_id", current_it_id_,
                "score", scoring_subsystem_.get_current_it_score()
            )
        );
    }
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

	else if (new_game_state == GameState::END_GAME) {
		gameobject_counter_->reset_counter();
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
                                    "value", (float)value / (1.5f * 32768));
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

            if (std::abs(value) < 6000) {
                value = 0;
            } else if(value < 0){
                value += 5000;
            } else {
                value -= 5000;
            }

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", (float)(value) / -32768);
            break;
        }

        case SDL_CONTROLLER_BUTTON_B: {
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::HAND_BRAKE,
                                    "value", (float)1);
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

    // Temporary, set first vehicle to be added as first it.
    // Assumes first vehicle object_id = 0.
    if (object_id.first == 0) {
        EventSystem::queue_event(
            Event(
                EventType::NEW_IT,
                "object_id", object_id.first
            )
        );
    }
}
void GameplaySystem::handle_object_transform_event(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    float x = e.get_value<float>("pos_x", true).first;
    float y = e.get_value<float>("pos_y", true).first;
    float z = e.get_value<float>("pos_z", true).first;

    object_positions_[object_id] = {x, y, z};
}

void GameplaySystem::handle_new_it(const Event& e) {
    int new_it_id = e.get_value<int>("object_id", true).first;
    current_it_id_ = new_it_id;
    scoring_subsystem_.set_new_it_id(new_it_id);
}

void GameplaySystem::handle_vehicle_collision(const Event& e) {
    int a_id = e.get_value<int>("a_id", true).first;
    int b_id = e.get_value<int>("b_id", true).first;
    std::cout << a_id << " collided with " << b_id << std::endl;

    std::vector<float> a_pos = object_positions_[a_id];
    std::vector<float> b_pos = object_positions_[b_id];


	std::vector<float> v_dir = { a_pos[0] - b_pos[0], a_pos[1] - b_pos[1], a_pos[2] - b_pos[2] };
	float magnitude = std::sqrt(v_dir[0] * v_dir[0] + v_dir[1] * v_dir[1] + v_dir[2] * v_dir[2]);

	v_dir[0] /= magnitude;
	v_dir[1] /= magnitude;
	v_dir[2] /= magnitude;

    // Apply knockback
    EventSystem::queue_event(
        Event(
            EventType::OBJECT_APPLY_FORCE,
            "object_id", a_id,
            // TODO: Pass glm::vec3 in events
            "x", v_dir[0] * 50000,
            "y", 100000.f,
            "z", v_dir[2] * 50000
        )
    );

    EventSystem::queue_event(
        Event(
            EventType::OBJECT_APPLY_FORCE,
            "object_id", b_id,
            // TODO: Pass glm::vec3 in events
            "x", -v_dir[0] * 50000,
            "y", 100000.f,
            "z", -v_dir[2] * 50000
        )
    );

    // Check for new it
    int new_it = -1;

    if (current_it_id_ == a_id) {
        new_it = b_id;
    } else if (current_it_id_ == b_id) {
        new_it = a_id;
    }

    if (new_it != -1) {
        EventSystem::queue_event(
            Event(
                EventType::NEW_IT,
                "object_id", new_it
            )
        );
    }
}

bool GameplaySystem::should_update_score() const {
    return  current_game_state_ ==  GameState::IN_GAME  &&
            current_it_id_      !=  -1;
}
