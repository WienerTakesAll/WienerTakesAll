#include <cmath>
#include <iostream>
#include <cmath>

#include "SDL.h"

#include "GameObjectCounter.h"
#include "GameplaySystem.h"
#include "VehicleControls.h"

namespace {
    const int MAX_SCORE = 2500;
    const int MAX_TRIGGER_VALUE = 32768;
    const float DRIVE_SPEED = 0.8f;
    const float BRAKE_SPEED = 0.8f;
    const glm::vec3 HOT_KNOCK_BACK_FORCE(50000.f, 200000.f, 50000.f);
}

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
    add_event_handler(EventType::ADD_POWERUP, &GameplaySystem::handle_add_powerup, this);
    add_event_handler(EventType::PICKUP_POWERUP, &GameplaySystem::handle_pickup_powerup, this);
    add_event_handler(EventType::CHANGE_POWERUP, &GameplaySystem::handle_change_powerup, this);
    add_event_handler(EventType::MOVE_POWERUP, &GameplaySystem::handle_move_powerup, this);
    add_event_handler(EventType::USE_POWERUP, &GameplaySystem::handle_use_powerup, this);

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

        int score_value = scoring_subsystem_.get_current_it_score();

        if (score_value < MAX_SCORE) {
            EventSystem::queue_event(
                Event(
                    EventType::UPDATE_SCORE,
                    "object_id", current_it_id_,
                    "score", score_value
                )
            );
        } else {
            EventSystem::queue_event(
                Event(
                    EventType::NEW_GAME_STATE,
                    "state", GameState::END_GAME,
                    "winner", current_it_id_
                )
            );
        }
    }

    if (current_game_state_ == GameState::IN_GAME) {
        powerup_subsystem_.update();

        // Move powerup here
        EventSystem::queue_event(
            Event(
                EventType::MOVE_POWERUP,
                "object_id", powerup_subsystem_.get_powerup_id(),
                "pos_x", 0.0f,
                "pos_y", 0.0f,
                "pos_z", 0.0f
            )
        );
    }
}

void GameplaySystem::handle_load(const Event& e) {
    powerup_subsystem_.load();
}

void GameplaySystem::handle_new_game_state(const Event& e) {
    GameState new_game_state = (GameState)e.get_value<int>("state", true).first;

    powerup_subsystem_.set_new_game_state(new_game_state);
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

        // Skybox
        EventSystem::queue_event(
            Event(
                EventType::ADD_SKYBOX,
                "object_id", gameobject_counter_->assign_id()
            )
        );

        // AI
        EventSystem::queue_event(
            Event(
                EventType::ACTIVATE_AI,
                "num_ai", 4 - num_humans
            )
        );

        // Powerup
        EventSystem::queue_event(
            Event(
                EventType::ADD_POWERUP,
                "object_id", gameobject_counter_->assign_id(),
                "type", static_cast<int>(powerup_subsystem_.get_next_powerup_type()),
                "pos_x", 0.0f,
                "pos_y", 1.5f,
                "pos_z", 2.0f
            )
        );

    } else if (new_game_state == GameState::START_MENU) {
        gameobject_counter_->reset_counter();
    }


    else if (new_game_state == GameState::END_GAME) {

        for (int i = 0; i < 4; ++i) {
            EventSystem::queue_event(
                Event(
                    EventType::OBJECT_APPLY_FORCE,
                    "object_id", i,
                    // TODO: Pass glm::vec3 in events
                    "x", 50000.f,
                    "y", 200000.f,
                    "z", 50000.f
                )
            );
        }
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
                                        "value", DRIVE_SPEED);
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
                                    "value", BRAKE_SPEED);
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

        // keyboard powerup
        case SDLK_SPACE: {
            new_events.emplace_back(EventType::USE_POWERUP,
                                    "index", player_id);
            break;
        }

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::FORWARD_DRIVE,
                                    "value", (float)value / MAX_TRIGGER_VALUE * DRIVE_SPEED);
            break;
        }

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::BRAKE,
                                    "value", (float)value / MAX_TRIGGER_VALUE);
            break;
        }

        case SDL_CONTROLLER_AXIS_LEFTX: {

            if (std::abs(value) < 6000) {
                value = 0;
            } else if (value < 0) {
                value += 5000;
            } else {
                value -= 5000;
            }

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", (float)(value) / -MAX_TRIGGER_VALUE);
            break;
        }

        case SDL_CONTROLLER_BUTTON_B: {
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::HAND_BRAKE,
                                    "value", 1.f);
            break;
        }

        case SDLK_ESCAPE: {
            new_events.emplace_back(EventType::NEW_GAME_STATE,
                                    "state", GameState::START_MENU);
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

    // A vehicle close enough to powerup should pick it up
    if (powerup_subsystem_.should_pickup_powerup(object_id, glm::vec3(x, y, z))) {
        EventSystem::queue_event(
            Event(
                EventType::PICKUP_POWERUP,
                "object_id", object_id,
                "powerup_id", powerup_subsystem_.get_powerup_id()
            )
        );
    }
}

void GameplaySystem::handle_new_it(const Event& e) {
    int new_it_id = e.get_value<int>("object_id", true).first;
    current_it_id_ = new_it_id;
    scoring_subsystem_.set_new_it_id(new_it_id);
}

void GameplaySystem::handle_add_powerup(const Event& e) {
    int powerup_id = e.get_value<int>("object_id", true).first;
    PowerupType new_type = static_cast<PowerupType>(e.get_value<int>("type", true).first);

    float x = e.get_value<float>("pos_x", true).first;
    float y = e.get_value<float>("pos_y", true).first;
    float z = e.get_value<float>("pos_z", true).first;
    glm::vec3 pos = glm::vec3(x, y, z);

    powerup_subsystem_.create_powerup(powerup_id, new_type, pos);
}

void GameplaySystem::handle_pickup_powerup(const Event& e) {
    // Pick up powerup
    int object_id = e.get_value<int>("object_id", true).first;
    int powerup_id = e.get_value<int>("powerup_id", true).first;
    powerup_subsystem_.pickup_powerup(object_id);

    // Change powerup type
    PowerupType new_type = powerup_subsystem_.get_next_powerup_type();
    EventSystem::queue_event(
        Event(
            EventType::CHANGE_POWERUP,
            "object_id", powerup_id,
            "type", static_cast<int>(new_type)
        )
    );

    // Move powerup to new location
    EventSystem::queue_event(
        Event(
            EventType::MOVE_POWERUP,
            "object_id", powerup_id,
            "pos_x", 2.0f,
            "pos_y", 0.0f,
            "pos_z", 0.0f
        )
    );
}

void GameplaySystem::handle_change_powerup(const Event& e) {
    PowerupType new_type = static_cast<PowerupType>(e.get_value<int>("type", true).first);
    powerup_subsystem_.change_powerup_type(new_type);
}

void GameplaySystem::handle_move_powerup(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;
    float x = e.get_value<float>("pos_x", true).first;
    float y = e.get_value<float>("pos_y", true).first;
    float z = e.get_value<float>("pos_z", true).first;
    powerup_subsystem_.move_powerup(object_id, glm::vec3(x, y, z));
}

void GameplaySystem::handle_use_powerup(const Event& e) {
    int object_id = e.get_value<int>("index", true).first;

    if (!powerup_subsystem_.can_use_powerup(object_id)) {
        std::cout << "pid: " << object_id << " cannot powerup" << std::endl;
        return;
    }

    PowerupType type = powerup_subsystem_.use_powerup(object_id);

    switch (type) {
        case PowerupType::KETCHUP:
            std::cout << "KETCHUP used by player " << object_id << std::endl;
            break;

        case PowerupType::PICKLE:
            std::cout << "PICKLE used by player " << object_id << std::endl;
            break;

        case PowerupType::HOT:
            for (int i = 0; i < 4; ++i) {
                if (i == object_id) {
                    continue;
                }

                EventSystem::queue_event(
                    Event(
                        EventType::OBJECT_APPLY_FORCE,
                        "object_id", i,
                        // TODO: Pass glm::vec3 in events
                        "x", HOT_KNOCK_BACK_FORCE.x,
                        "y", HOT_KNOCK_BACK_FORCE.y,
                        "z", HOT_KNOCK_BACK_FORCE.z
                    )
                );
            }

            break;

        case PowerupType::NONE:
        default:
            break;
    }

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
