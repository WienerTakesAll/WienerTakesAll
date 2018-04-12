#include <cmath>
#include <iostream>

#include "SDL.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "GameObjectCounter.h"
#include "GameplaySystem.h"
#include "VehicleControls.h"
#include "StatusEffect.h"

namespace {
    const int MAX_SCORE = 2500;
    const int MAX_TRIGGER_VALUE = 32768;
    const float DRIVE_SPEED = 0.6f;
    const float BRAKE_SPEED = 0.8f;
    const float STEER_DAMPENING = 0.6f;
    const float KEYBOARD_STEER_AMOUNT = 0.4f;
    const glm::vec3 COLLISION_KNOCK_BACK_FORCE(15000.f, 80000.f, 15000.f);
    const int NUM_MOUNDS = 20;
    const int SPAWN_DISTANCE_FROM_CENTER = 20;

    const int GOOD_KETCHUP_DURATION = 180;
    const int BAD_KETCHUP_DURATION = 180;
    const float GOOD_KETCHUP_BOOST = 1200.0f;
    const float BAD_KETCHUP_BOOST = 5000.0f;
    const int MUSTARD_EFFECT_DURATION = 60;
    const glm::vec3 MUSTARD_KNOCK_BACK_FORCE(15000.f, 80000.f, 15000.f);
    const int DOMINATED_DURATION = 180;;
    const int CONTROLS_REVERSED_DURATION = 180;;
    const int INVINCIBILITY_DURATION = 180;
}

GameplaySystem::GameplaySystem()
    : gameobject_counter_(GameObjectCounter::get_instance())
    , current_game_state_(GameState::START_MENU)
    , current_it_id_(-1)
    , dominator_(-1) {
    add_event_handler(EventType::LOAD_EVENT, &GameplaySystem::handle_load, this);
    add_event_handler(EventType::KEYPRESS_EVENT, &GameplaySystem::handle_key_press, this);
    add_event_handler(EventType::NEW_GAME_STATE, &GameplaySystem::handle_new_game_state, this);
    add_event_handler(EventType::ADD_VEHICLE, &GameplaySystem::handle_add_vehicle, this);
    add_event_handler(EventType::OBJECT_TRANSFORM_EVENT, &GameplaySystem::handle_object_transform_event, this);
    add_event_handler(EventType::VEHICLE_COLLISION, &GameplaySystem::handle_vehicle_collision, this);
    add_event_handler(EventType::NEW_IT, &GameplaySystem::handle_new_it, this);
    add_event_handler(EventType::ADD_POWERUP, &GameplaySystem::handle_add_powerup, this);
    add_event_handler(EventType::USE_POWERUP, &GameplaySystem::handle_use_powerup, this);
    add_event_handler(EventType::NEW_STATUS_EFFECT, &GameplaySystem::handle_new_status_effect, this);
    add_event_handler(EventType::PICKUP_POWERUP, &GameplaySystem::handle_pickup_powerup, this);
    add_event_handler(EventType::CHANGE_POWERUP, &GameplaySystem::handle_change_powerup, this);
    add_event_handler(EventType::PLAYER_FELL_OFF_ARENA, &GameplaySystem::handle_player_fell_off_arena, this);
    add_event_handler(EventType::ADD_CHARCOAL, &GameplaySystem::handle_add_charcoal, this);

    controllers_reversed_.fill(false);

    EventSystem::queue_event(
        Event(
            EventType::NEW_GAME_STATE,
            "state", GameState::START_MENU
        )
    );
}

void GameplaySystem::update() {
    if (current_game_state_ != GameState::IN_GAME) {
        // No gameplay updates if not in game!
        return;
    }

    if (current_it_id_ == -1) {
        // Game not done initializing
        return;
    }


    // update scoring subsystem
    scoring_subsystem_.update();
    int score_value = scoring_subsystem_.get_current_it_score();
    int score_lock = scoring_subsystem_.get_current_score_lock_frames();
    EventSystem::queue_event(
        Event(
            EventType::UPDATE_SCORE,
            "object_id", current_it_id_,
            "score", score_value,
            "lock_frames", score_lock
        )
    );

    if (score_value >= MAX_SCORE) {
        EventSystem::queue_event(
            Event(
                EventType::NEW_GAME_STATE,
                "state", GameState::END_GAME,
                "winner", current_it_id_
            )
        );
        // Game is over
        return;
    }

    // update powerup subsystem
    powerup_subsystem_.update();

    if (object_positions_.find(powerup_subsystem_.get_powerup_id()) != object_positions_.end()) {
        glm::vec3 powerup_cur_loc = object_positions_[powerup_subsystem_.get_powerup_id()];

        if (powerup_subsystem_.should_update_powerup_position(powerup_subsystem_.get_powerup_id(), powerup_cur_loc)) {
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
        }
    }

    // update status effects
    for (auto& player_status : player_status_effects_) {
        if (player_status.second.duration_ <= 0 ||
                player_status.second.effect_ == StatusEffect::NONE) {
            continue;
        }

        // Apply effects for continuous status effects
        // Ketchup effect
        if (player_status.second.effect_ == StatusEffect::GOOD_KETCHUP ||
                player_status.second.effect_ == StatusEffect::BAD_KETCHUP) {

            float boost = GOOD_KETCHUP_BOOST;

            if (player_status.second.effect_ == StatusEffect::BAD_KETCHUP) {
                boost = BAD_KETCHUP_BOOST;
            }

            glm::vec3 boost_direction = object_rotations_[player_status.first] * glm::vec3(0.0f, 0.0f, boost);
            EventSystem::queue_event(
                Event(
                    EventType::OBJECT_APPLY_FORCE,
                    "object_id", player_status.first,
                    "x", boost_direction.x,
                    "y", boost_direction.y,
                    "z", boost_direction.z
                )
            );
        }

        // update status effects
        if (--player_status.second.duration_ <= 0) {
            player_status.second.effect_ = StatusEffect::NONE;
            EventSystem::queue_event(
                Event(
                    EventType::NEW_STATUS_EFFECT,
                    "type", StatusEffect::NONE,
                    "object_id", player_status.first
                )
            );
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
        int first_player_id = gameobject_counter_->assign_id();
        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", first_player_id,
                // TODO: Pass glm::vec3 in events
                "pos_x", 0,
                "pos_y", 10,
                "pos_z", -SPAWN_DISTANCE_FROM_CENTER,
                "rotation", 0.0f
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", gameobject_counter_->assign_id(),
                // TODO: Pass glm::vec3 in events
                "pos_x", -SPAWN_DISTANCE_FROM_CENTER,
                "pos_y", 10,
                "pos_z", 0,
                "rotation", 90.0f
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", gameobject_counter_->assign_id(),
                // TODO: Pass glm::vec3 in events
                "pos_x", 0,
                "pos_y", 10,
                "pos_z", SPAWN_DISTANCE_FROM_CENTER,
                "rotation", 180.0f
            )
        );

        int last_player_id = gameobject_counter_->assign_id();
        EventSystem::queue_event(
            Event(
                EventType::ADD_VEHICLE,
                "object_id", last_player_id,
                // TODO: Pass glm::vec3 in events
                "pos_x", SPAWN_DISTANCE_FROM_CENTER,
                "pos_y", 10,
                "pos_z", 0,
                "rotation", 270.0f
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::NEW_IT,
                "object_id", (rand() % (last_player_id - first_player_id + 1)) + first_player_id
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

        // mounds
        for (size_t i = 0; i < NUM_MOUNDS; i++) {
            EventSystem::queue_event(
                Event(
                    EventType::ADD_CHARCOAL,
                    "object_id", gameobject_counter_->assign_id(),
                    // TODO: Pass glm::vec3 in events
                    "pos_x", (rand() % 140) - 70,
                    "pos_y", (rand() % 3),
                    "pos_z", (rand() % 140) - 70
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
        // glm::vec3 powerup_loc = powerup_subsystem_.get_next_powerup_position();

        int powerup_id = powerup_subsystem_.get_powerup_id();

        if (powerup_id == -1) {
            powerup_id = gameobject_counter_->assign_id();
        }

        EventSystem::queue_event(
            Event(
                EventType::ADD_POWERUP,
                "object_id", powerup_id,
                "type", static_cast<int>(powerup_subsystem_.get_next_powerup_type()),
                "pos_x", 0.f,
                "pos_y", 2.7f,
                "pos_z", 0.f
            )
        );

        EventSystem::queue_event(
            Event(
                EventType::OBJECT_TRANSFORM_EVENT,
                "object_id", powerup_id,
                "pos_x", 0.f,
                "pos_y", 2.7f,
                "pos_z", 0.f,
                "qua_x", 0.f,
                "qua_y", 0.0f,
                "qua_z", 0.f,
                "qua_w", 1.0f
            )
        );

    } else if (new_game_state == GameState::START_MENU) {
        gameobject_counter_->reset_counter();
        dominator_ = -1;
        controllers_reversed_.fill(false);
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

    if (controllers_reversed_[player_id]) {
        key = get_reverse_key(key);
    }

    std::vector<Event> new_events;

    if (player_status_effects_[player_id].effect_ == StatusEffect::DOMINATED) {
        return;
    }

    switch (key) {
        // self powerup
        case SDLK_q: // fall through
        case SDLK_r:
        case SDLK_u:
        case SDLK_RSHIFT:
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            if ((value == SDL_KEYDOWN || value == SDL_CONTROLLERBUTTONDOWN) && powerup_subsystem_.can_use_powerup(player_id)) {
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
            if ((value == SDL_KEYDOWN || value == SDL_CONTROLLERBUTTONDOWN) && powerup_subsystem_.can_use_powerup(player_id)) {
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
                                        "value", calculate_player_speed(player_id));
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
            if (value == SDL_KEYDOWN) {
                new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                        "index", player_id,
                                        "type", VehicleControlType::BRAKE,
                                        "value", BRAKE_SPEED);
            } else if (value == SDL_KEYUP) {
                new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                        "index", player_id,
                                        "type", VehicleControlType::BRAKE,
                                        "value", 0.f);
            }

            break;

        // keyboard left steer
        case SDLK_a: // fall through
        case SDLK_f:
        case SDLK_j:
        case SDLK_LEFT: {
            float steer_amount = KEYBOARD_STEER_AMOUNT;
            float steer_value = value != SDL_KEYUP ? steer_amount : 0.f;
            steer_value = std::max(-1.0f, std::min(1.0f, steer_value));

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", steer_value);
            break;
        }

        // keyboard right steer
        case SDLK_d: // fall through
        case SDLK_h:
        case SDLK_l:
        case SDLK_RIGHT: {
            float steer_amount = -KEYBOARD_STEER_AMOUNT;

            float steer_value = value != SDL_KEYUP ? steer_amount : 0.f;
            steer_value = std::max(-1.0f, std::min(1.0f, steer_value));

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", steer_value);

            break;
        }

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::FORWARD_DRIVE,
                                    "value", (float)(value) / MAX_TRIGGER_VALUE * calculate_player_speed(player_id));

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

            float steer_dampening = STEER_DAMPENING;

            float steer_value = (float)(value * steer_dampening) / -MAX_TRIGGER_VALUE;
            steer_value = std::max(-1.0f, std::min(1.0f, steer_value));

            new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                    "index", player_id,
                                    "type", VehicleControlType::STEER,
                                    "value", steer_value);
            break;
        }

        case SDLK_ESCAPE:
            new_events.emplace_back(EventType::NEW_GAME_STATE,
                                    "state", GameState::START_MENU);
            break;

        default:
            return;
    }

    // if this input is the current dominator's input
    if (player_id == dominator_) {
        for (int other_player = 0; other_player < 4; other_player++) {
            // forward this input to all dominated players
            if (other_player == player_id ||
                    player_status_effects_[other_player].effect_ != DOMINATED) {
                continue;
            }

            auto new_events_copy = new_events;

            for (const auto& event : new_events_copy) {
                if (event.event_type != EventType::VEHICLE_CONTROL) {
                    continue;
                }

                std::cout << "DOMINATING" << std::endl;
                new_events.emplace_back(EventType::VEHICLE_CONTROL,
                                        "index", other_player,
                                        "type", event.get_value<int>("type", true).first,
                                        "value", event.get_value<float>("value", true).first);
            }
        }
    }

    for (const auto& event : new_events) {
        EventSystem::queue_event(Event(event));
    }
}

void GameplaySystem::handle_add_vehicle(const Event& e) {
    std::pair<int, bool> object_id = e.get_value<int>("object_id", true);
    scoring_subsystem_.add_vehicle(object_id.first);

    PlayerStatusEffect effect_data;
    effect_data.effect_ = StatusEffect::NONE;
    effect_data.duration_ = 0;
    player_status_effects_.emplace(object_id.first, effect_data);
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
                "powerup_id", powerup_subsystem_.get_powerup_id(),
                "powerup_type", powerup_subsystem_.get_powerup_type()
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
    scoring_subsystem_.set_new_it_id(new_it_id);
    current_it_id_ = new_it_id;

    EventSystem::queue_event(
        Event(
            EventType::NEW_STATUS_EFFECT,
            "type", StatusEffect::INVINCIBILITY,
            "object_id", new_it_id
        )
    );
}

void GameplaySystem::handle_add_charcoal(const Event& e) {
    std::pair<int, bool> x = e.get_value<int>("pos_x", true);
    std::pair<int, bool> y = e.get_value<int>("pos_y", true);
    std::pair<int, bool> z = e.get_value<int>("pos_z", true);

    powerup_subsystem_.add_mound_location(x.first, y.first, z.first);
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

void GameplaySystem::handle_new_status_effect(const Event& e) {
    StatusEffect new_effect = static_cast<StatusEffect>(e.get_value<int>("type", true).first);
    int player_id = e.get_value<int>("object_id", true).first;

    if (player_status_effects_[player_id].effect_ == StatusEffect::INVINCIBILITY) {
        if (new_effect != StatusEffect::INVINCIBILITY) {
            return;
        }
    }

    controllers_reversed_[player_id] = false;

    switch (new_effect) {
        case StatusEffect::NONE:
            player_status_effects_[player_id].effect_ = StatusEffect::NONE;
            player_status_effects_[player_id].duration_ = 0;
            break;

        case StatusEffect::BAD_KETCHUP:
            player_status_effects_[player_id].effect_ = StatusEffect::BAD_KETCHUP;
            player_status_effects_[player_id].duration_ = BAD_KETCHUP_DURATION;
            break;

        case StatusEffect::GOOD_KETCHUP:
            player_status_effects_[player_id].effect_ = StatusEffect::GOOD_KETCHUP;
            player_status_effects_[player_id].duration_ = GOOD_KETCHUP_DURATION;
            break;

        case StatusEffect::MUSTARD_EFFECT:
            player_status_effects_[player_id].effect_ = StatusEffect::MUSTARD_EFFECT;
            player_status_effects_[player_id].duration_ = MUSTARD_EFFECT_DURATION;
            break;

        case StatusEffect::INVINCIBILITY:
            player_status_effects_[player_id].effect_ = StatusEffect::INVINCIBILITY;
            player_status_effects_[player_id].duration_ = INVINCIBILITY_DURATION;
            break;

        case StatusEffect::DOMINATED:
            player_status_effects_[player_id].effect_ = StatusEffect::DOMINATED;
            player_status_effects_[player_id].duration_ = DOMINATED_DURATION;
            break;

        case StatusEffect::CONTROLS_REVERSED:
            player_status_effects_[player_id].effect_ = StatusEffect::CONTROLS_REVERSED;
            player_status_effects_[player_id].duration_ = CONTROLS_REVERSED_DURATION;
            controllers_reversed_[player_id] = true;
            break;

        default:
            assert(false);
            break;
    }
}


void GameplaySystem::handle_use_powerup(const Event& e) {
    int object_id = e.get_value<int>("index", true).first;
    int type = e.get_value<int>("type", true).first;
    int target = e.get_value<int>("target", true).first;

    switch (type) {
        case PowerupType::KETCHUP: {
            std::cout << "KETCHUP used by player " << object_id << std::endl;
            powerup_subsystem_.spend_powerup(object_id);

            if (target == PowerupTarget::SELF) {

                EventSystem::queue_event(
                    Event(
                        EventType::NEW_STATUS_EFFECT,
                        "type", StatusEffect::GOOD_KETCHUP,
                        "object_id", object_id
                    )
                );
            } else {
                for (auto& player_effect : player_status_effects_) {
                    if (player_effect.first == object_id) {
                        continue;
                    }


                    EventSystem::queue_event(
                        Event(
                            EventType::NEW_STATUS_EFFECT,
                            "type", StatusEffect::BAD_KETCHUP,
                            "object_id", player_effect.first
                        )
                    );
                }
            }

            break;
        }

        case PowerupType::MUSTARD: {
            std::cout << "MUSTARD used by player " << object_id << std::endl;
            powerup_subsystem_.spend_powerup(object_id);

            if (target == PowerupTarget::SELF) {

                EventSystem::queue_event(
                    Event(
                        EventType::OBJECT_APPLY_FORCE,
                        "object_id", object_id,
                        "x", 0.0f,
                        "y", MUSTARD_KNOCK_BACK_FORCE.y,
                        "z", 0.f
                    )
                );

                EventSystem::queue_event(
                    Event(
                        EventType::NEW_STATUS_EFFECT,
                        "type", StatusEffect::MUSTARD_EFFECT,
                        "object_id", object_id
                    )
                );
            } else {
                for (auto& player_effect : player_status_effects_) {
                    if (player_effect.first == object_id) {
                        continue;
                    }


                    EventSystem::queue_event(
                        Event(
                            EventType::OBJECT_APPLY_FORCE,
                            "object_id", player_effect.first,
                            "x", MUSTARD_KNOCK_BACK_FORCE.x,
                            "y", MUSTARD_KNOCK_BACK_FORCE.y,
                            "z", MUSTARD_KNOCK_BACK_FORCE.z
                        )
                    );

                    EventSystem::queue_event(
                        Event(
                            EventType::NEW_STATUS_EFFECT,
                            "type", StatusEffect::MUSTARD_EFFECT,
                            "object_id", player_effect.first
                        )
                    );
                }
            }
        }
        break;

        case PowerupType::RELISH: {
            std::cout << "RELISH used by player " << object_id << std::endl;
            powerup_subsystem_.spend_powerup(object_id);
            StatusEffect new_effect = StatusEffect::NONE;

            if (target == PowerupTarget::SELF) {
                dominator_ = object_id;
                new_effect = StatusEffect::DOMINATED;
            } else {
                new_effect = StatusEffect::CONTROLS_REVERSED;
            }

            for (auto& player_effect : player_status_effects_) {
                if (player_effect.first == object_id) {
                    continue;
                }


                EventSystem::queue_event(
                    Event(
                        EventType::NEW_STATUS_EFFECT,
                        "type", new_effect,
                        "object_id", player_effect.first
                    )
                );
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

    bool a_invincible = player_status_effects_[a_id].effect_ == StatusEffect::INVINCIBILITY;
    bool b_invincible = player_status_effects_[b_id].effect_ == StatusEffect::INVINCIBILITY;

    glm::vec3 a_pos = object_positions_[a_id];
    glm::vec3 b_pos = object_positions_[b_id];
    glm::vec3 v_dir = glm::normalize(a_pos - b_pos);

    // Apply knockback
    if (!a_invincible) {
        EventSystem::queue_event(
            Event(
                EventType::OBJECT_APPLY_FORCE,
                "object_id", a_id,
                "x", v_dir[0] * COLLISION_KNOCK_BACK_FORCE.x,
                "y", COLLISION_KNOCK_BACK_FORCE.y,
                "z", v_dir[2] * COLLISION_KNOCK_BACK_FORCE.z
            )
        );
    }

    if (!b_invincible) {
        EventSystem::queue_event(
            Event(
                EventType::OBJECT_APPLY_FORCE,
                "object_id", b_id,
                "x", -v_dir[0] * COLLISION_KNOCK_BACK_FORCE.x,
                "y", COLLISION_KNOCK_BACK_FORCE.y,
                "z", -v_dir[2] * COLLISION_KNOCK_BACK_FORCE.z
            )
        );
    }

    // Check for new it
    int new_it = -1;

    if (current_it_id_ == a_id && !a_invincible) {
        new_it = b_id;
    } else if (current_it_id_ == b_id && !b_invincible) {
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


float GameplaySystem::calculate_player_speed(int player) {
    float average_score = 0;

    for (size_t i = 0; i < 4; i++) {
        average_score += scoring_subsystem_.get_player_score(i);
    }

    average_score *= 0.25f;
    float player_score = scoring_subsystem_.get_player_score(player);
    float speed_penalty = 1.f + ((average_score - player_score) * 0.02f);
    speed_penalty = std::max(1.f, std::min(1.f, speed_penalty));

    float total_speed = speed_penalty * DRIVE_SPEED * std::sqrt((4.f - object_velocities_[player].length())) * 0.25f;
    return std::max(0.f, std::min(1.f, total_speed));
}

void GameplaySystem::handle_player_fell_off_arena(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    if (object_id != current_it_id_ ||
            player_status_effects_[object_id].effect_ == StatusEffect::INVINCIBILITY) {
        return;
    }

    int losing_player = 0;
    int losing_player_score = scoring_subsystem_.get_player_score(losing_player);

    for (int i = 1; i < 4; i++) { // players 1-3
        int player_score = scoring_subsystem_.get_player_score(i);

        if (player_score < losing_player_score) {
            losing_player = i;
            losing_player_score = player_score;
        }
    }

    EventSystem::queue_event(
        Event(
            EventType::NEW_IT,
            "object_id", losing_player
        )
    );
}

const int GameplaySystem::get_reverse_key(const int key) const {
    int ret = -1;

    switch (key) {
        // self powerup
        case SDLK_q:
        case SDLK_r:
        case SDLK_u:
        case SDLK_RSHIFT:
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            ret = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
            break;

        // others powerup
        case SDLK_e:
        case SDLK_y:
        case SDLK_o:
        case SDLK_RETURN:
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            ret = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
            break;

        // Keyboard acceleration
        case SDLK_w:
        case SDLK_t:
        case SDLK_i:
        case SDLK_UP:
            ret = SDLK_DOWN;
            break;

        // keyboard braking
        case SDLK_s:
        case SDLK_g:
        case SDLK_k:
        case SDLK_DOWN:
            ret = SDLK_UP;
            break;

        // keyboard left steer
        case SDLK_a:
        case SDLK_f:
        case SDLK_j:
        case SDLK_LEFT:
            ret = SDLK_RIGHT;
            break;

        // keyboard right steer
        case SDLK_d:
        case SDLK_h:
        case SDLK_l:
        case SDLK_RIGHT:
            ret = SDLK_LEFT;
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            ret = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
            break;

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            ret = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
            break;

        case SDL_CONTROLLER_AXIS_LEFTX: {
            ret = SDL_CONTROLLER_AXIS_RIGHTX;
            break;
        }

        default:
            ret = -1;
    }

    return ret;
}
