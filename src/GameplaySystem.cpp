#include <cmath>
#include <iostream>

#include "SDL.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "GameObjectCounter.h"
#include "GameplaySystem.h"
#include "VehicleControls.h"

namespace {
    const int MAX_SCORE = 2500;
    const int MAX_TRIGGER_VALUE = 32768;
    const float DRIVE_SPEED = 0.6f;
    const float BRAKE_SPEED = 0.8f;
    const float KETCHUP_BOOST = 1500.0f;
    const float NORMAL_STEER_DAMPENING = 0.6f;
    const float RELISH_STEER_DAMPENING = 1.0f;
    const glm::vec3 HOT_KNOCK_BACK_FORCE(15000.f, 80000.f, 15000.f);
    const float NORMAL_KEYBOARD_STEER_AMOUNT = 0.4f;
    const float RELISH_KEYBOARD_STEER_AMOUNT = 0.4f;
    const glm::vec3 COLLISION_KNOCK_BACK_FORCE(15000.f, 80000.f, 15000.f);
    const float RELISH_DURATION = 2.5f;
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
    add_event_handler(EventType::USE_POWERUP, &GameplaySystem::handle_use_powerup, this);
    add_event_handler(EventType::PICKUP_POWERUP, &GameplaySystem::handle_pickup_powerup, this);
    add_event_handler(EventType::CHANGE_POWERUP, &GameplaySystem::handle_change_powerup, this);

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

        if (object_positions_.find(powerup_subsystem_.get_powerup_id()) == object_positions_.end() ||
                !powerup_subsystem_.should_update_powerup_position(powerup_subsystem_.get_powerup_id())) {
            return;
        }

        // Move powerup here
        glm::vec3 powerup_cur_loc = object_positions_[powerup_subsystem_.get_powerup_id()];
        EventSystem::queue_event(
            Event(
                EventType::OBJECT_TRANSFORM_EVENT,
                "object_id", powerup_subsystem_.get_powerup_id(),
                "pos_x", powerup_cur_loc.x,
                "pos_y", powerup_cur_loc.y,
                "pos_z", powerup_cur_loc.z,
                "qua_w", 1.0f,
                "qua_x", 0.0f,
                "qua_y", 0.0f,
                "qua_z", 0.0f
            )
        );



        for (auto& powerup_data : powerup_datas_) {
            if (powerup_data.second.ketchup > 0.f) {
                glm::vec3 boost_direction
                    = object_rotations_[powerup_data.first] * glm::vec3(0.0f, 0.0f, powerup_data.second.ketchup * KETCHUP_BOOST);
                EventSystem::queue_event(
                    Event(
                        EventType::OBJECT_APPLY_FORCE,
                        "object_id", powerup_data.first,
                        "x", boost_direction.x,
                        "y", boost_direction.y,
                        "z", boost_direction.z
                    ));

                powerup_data.second.ketchup -= 0.01f;
            }

            if (powerup_data.second.relish > 0.f) {
                powerup_data.second.relish -= 0.01f;
            }
        }
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
                "pos_z", 0
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", gameobject_counter_->assign_id(),
                // TODO: Pass glm::vec3 in events
                "pos_x", 10,
                "pos_y", 10,
                "pos_z", 0//,
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", gameobject_counter_->assign_id(),
                // TODO: Pass glm::vec3 in events
                "pos_x", -4,
                "pos_y", 2,
                "pos_z", 0
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", gameobject_counter_->assign_id(),
                // TODO: Pass glm::vec3 in events
                "pos_x", -10,
                "pos_y", 2,
                "pos_z", 0
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

        //CHARCOAL_TEST
        for (size_t i = 0; i < 10; i++) {
            EventSystem::queue_event(
                Event(
                    EventType::ADD_CHARCOAL,
                    "object_id", gameobject_counter_->assign_id(),
                    // TODO: Pass glm::vec3 in events
                    "pos_x", (rand() % 150) - 75,
                    "pos_y", 1,
                    "pos_z", (rand() % 150) - 75//,
                )
            );
        }

        // AI
        EventSystem::queue_event(
            Event(
                EventType::ACTIVATE_AI,
                "num_ai", 4 - num_humans
            )
        );

        // Powerup
        glm::vec3 powerup_loc = powerup_subsystem_.get_next_powerup_position();
        EventSystem::queue_event(
            Event(
                EventType::ADD_POWERUP,
                "object_id", gameobject_counter_->assign_id(),
                "type", static_cast<int>(powerup_subsystem_.get_next_powerup_type()),
                "pos_x", powerup_loc.x,
                "pos_y", powerup_loc.y,
                "pos_z", powerup_loc.z
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
        // self powerup
        case SDLK_q: // fall through
        case SDLK_r:
        case SDLK_u:
        case SDLK_RSHIFT:
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            if (powerup_subsystem_.can_use_powerup(player_id)) {
                new_events.emplace_back(EventType::USE_POWERUP,
                                        "type", powerup_subsystem_.get_player_powerup_type(player_id),
                                        "target", PowerupTarget::SELF,
                                        "index", player_id);
            }

            break;

        // others powerup
        case SDLK_e: // fall through
        case SDLK_y:
        case SDLK_o:
        case SDLK_RETURN:
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            if (powerup_subsystem_.can_use_powerup(player_id)) {
                new_events.emplace_back(EventType::USE_POWERUP,
                                        "type", powerup_subsystem_.get_player_powerup_type(player_id),
                                        "target", PowerupTarget::OTHERS,
                                        "index", player_id);
            }

            break;

        // Keyboard acceleration
        case SDLK_w: // fall through
        case SDLK_t:
        case SDLK_i:
        case SDLK_UP:
            if (value == SDL_KEYDOWN) {
                new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                        "index", player_id,
                                        "type", VehicleControlType::FORWARD_DRIVE,
                                        "value", calculatePlayerSpeed(player_id));
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

        // keyboard braking
        case SDLK_s: // fall through
        case SDLK_g:
        case SDLK_k:
        case SDLK_DOWN:
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::FORWARD_DRIVE,
                                    "value", 0.0f);
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::BRAKE,
                                    "value", BRAKE_SPEED);
            break;

        // keyboard left steer
        case SDLK_a: // fall through
        case SDLK_f:
        case SDLK_j:
        case SDLK_LEFT: {
            float steer_amount = NORMAL_KEYBOARD_STEER_AMOUNT;

            if (powerup_datas_[player_id].relish > 0.f) {
                steer_amount = RELISH_KEYBOARD_STEER_AMOUNT;
            }

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", value != SDL_KEYUP ? steer_amount : 0.f);

            break;
        }

        // keyboard right steer
        case SDLK_d: // fall through
        case SDLK_h:
        case SDLK_l:
        case SDLK_RIGHT: {
            float steer_amount = -NORMAL_KEYBOARD_STEER_AMOUNT;

            if (powerup_datas_[player_id].relish > 0.f) {
                steer_amount = RELISH_KEYBOARD_STEER_AMOUNT;
            }

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", value != SDL_KEYUP ? steer_amount : 0.f);
            break;
        }

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::FORWARD_DRIVE,
                                    "value", (float)(value) / MAX_TRIGGER_VALUE * calculatePlayerSpeed(player_id));
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::BRAKE,
                                    "value", (float)(value) / MAX_TRIGGER_VALUE);
            break;

        case SDL_CONTROLLER_AXIS_LEFTX: {

            if (std::abs(value) < 6000) { // DEADZONE
                value = 0;
            } else if (value < 0) {
                value += 5000;
            } else {
                value -= 5000;
            }

            float steer_dampening = NORMAL_STEER_DAMPENING;

            if (powerup_datas_[player_id].relish > 0.f) {
                steer_dampening = RELISH_STEER_DAMPENING;
            }

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", (float)(value * steer_dampening)  / -MAX_TRIGGER_VALUE);
            break;
        }

        case SDLK_ESCAPE:
            new_events.emplace_back(EventType::NEW_GAME_STATE,
                                    "state", GameState::START_MENU);
            break;

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

    player_powerup_data data;
    data.ketchup = 0.f;
    powerup_datas_.emplace(object_id.first, data);

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
    float qw = e.get_value<float>("qua_w", true).first;
    float qx = e.get_value<float>("qua_x", true).first;
    float qy = e.get_value<float>("qua_y", true).first;
    float qz = e.get_value<float>("qua_z", true).first;

    object_rotations_[object_id] = glm::quat(qw, qx, qy, qz);
    object_positions_[object_id] = glm::vec3(x, y, z);

    if (powerup_subsystem_.is_powerup(object_id)) {
        powerup_subsystem_.change_powerup_position(object_id, glm::vec3(x, y, z));
    }

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

    if (object_id < 4 && object_id != current_it_id_) {
        glm::vec3 vec_to = glm::normalize(object_positions_[current_it_id_] - object_positions_[object_id]);
        glm::mat4 rot_matrix = glm::toMat4(object_rotations_[object_id]);
        glm::vec4 vec_to_it = glm::vec4(vec_to, 0.f) * rot_matrix;

        EventSystem::queue_event(
            Event(
                EventType::UPDATE_DIRECTION_TO_IT,
                "object_id", object_id,
                "x", vec_to_it.x,
                "y", vec_to_it.y,
                "z", vec_to_it.z
            )
        );
    }


    auto e_vx = e.get_value<float>("vel_x", false);

    if (e_vx.second) {
        float vx = e_vx.first;
        float vy = e.get_value<float>("vel_y", true).first;
        float vz = e.get_value<float>("vel_z", true).first;

        glm::vec3 velocity(vx, vy, vz);
        object_velocities_[object_id] = velocity;
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
    glm::vec3 new_location = powerup_subsystem_.get_next_powerup_position();
    EventSystem::queue_event(
        Event(
            EventType::OBJECT_TRANSFORM_EVENT,
            "object_id", powerup_id,
            "pos_x", new_location.x,
            "pos_y", new_location.y,
            "pos_z", new_location.z,
            "qua_w", 1.0f,
            "qua_x", 0.0f,
            "qua_y", 0.0f,
            "qua_z", 0.0f
        )
    );
}

void GameplaySystem::handle_change_powerup(const Event& e) {
    PowerupType new_type = static_cast<PowerupType>(e.get_value<int>("type", true).first);
    powerup_subsystem_.change_powerup_type(new_type);
}

void GameplaySystem::handle_use_powerup(const Event& e) {
    int object_id = e.get_value<int>("index", true).first;
    int type = e.get_value<int>("type", true).first;
    int target = e.get_value<int>("target", true).first;

    powerup_subsystem_.spend_powerup(object_id);

    switch (type) {
        case PowerupType::KETCHUP: {
            std::cout << "KETCHUP used by player " << object_id << std::endl;

            if (target == PowerupTarget::SELF) {
                powerup_datas_[object_id].ketchup = 1.0f;
            } else {
                for (auto& powerup_data : powerup_datas_) {
                    if (powerup_data.first != object_id) {
                        powerup_data.second.ketchup = 2.5f;
                    }
                }
            }

            break;
        }

        case PowerupType::MUSTARD:
            std::cout << "MUSTARD used by player " << object_id << std::endl;

            if (target == PowerupTarget::SELF) {
                EventSystem::queue_event(
                    Event(
                        EventType::OBJECT_APPLY_FORCE,
                        "object_id", object_id,
                        // TODO: Pass glm::vec3 in events
                        "x", HOT_KNOCK_BACK_FORCE.x * 0.0f,
                        "y", HOT_KNOCK_BACK_FORCE.y,
                        "z", HOT_KNOCK_BACK_FORCE.z * 0.f
                    ));
            } else {
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
                        ));
                }
            }

        case PowerupType::RELISH:
            std::cout << "RELISH used by player " << object_id << std::endl;

            if (target == PowerupTarget::SELF) {
                powerup_datas_[object_id].relish = RELISH_DURATION;
            } else {
                for (auto& powerup_data : powerup_datas_) {
                    if (powerup_data.first != object_id) {
                        powerup_data.second.relish = RELISH_DURATION;
                    }
                }
            }

            break;

        default:
            break;
    }
}

void GameplaySystem::handle_vehicle_collision(const Event& e) {
    int a_id = e.get_value<int>("a_id", true).first;
    int b_id = e.get_value<int>("b_id", true).first;
    std::cout << a_id << " collided with " << b_id << std::endl;

    glm::vec3 a_pos = object_positions_[a_id];
    glm::vec3 b_pos = object_positions_[b_id];
    glm::vec3 v_dir = glm::normalize(a_pos - b_pos);

    // Apply knockback
    EventSystem::queue_event(
        Event(
            EventType::OBJECT_APPLY_FORCE,
            "object_id", a_id,
            // TODO: Pass glm::vec3 in events
            "x", v_dir[0] * COLLISION_KNOCK_BACK_FORCE.x,
            "y", COLLISION_KNOCK_BACK_FORCE.y,
            "z", v_dir[2] * COLLISION_KNOCK_BACK_FORCE.z
        )
    );

    EventSystem::queue_event(
        Event(
            EventType::OBJECT_APPLY_FORCE,
            "object_id", b_id,
            // TODO: Pass glm::vec3 in events
            "x", -v_dir[0] * COLLISION_KNOCK_BACK_FORCE.x,
            "y", COLLISION_KNOCK_BACK_FORCE.y,
            "z", -v_dir[2] * COLLISION_KNOCK_BACK_FORCE.z
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


float GameplaySystem::calculatePlayerSpeed(int player) {
    float averageScore = 0;

    for (size_t i = 0; i < 4; i++) {
        averageScore += scoring_subsystem_.get_player_score(i);
    }

    averageScore *= 0.25f;
    float playerScore = scoring_subsystem_.get_player_score(player);
    float speedPenalty = 1.f + ((averageScore - playerScore) * 0.02f);
    speedPenalty = std::max(1.f, std::min(1.f, speedPenalty));

    float totalSpeed = speedPenalty * DRIVE_SPEED * std::sqrt((4.f - object_velocities_[player].length())) * 0.25f;
    return std::max(0.f, std::min(1.f, totalSpeed));
}