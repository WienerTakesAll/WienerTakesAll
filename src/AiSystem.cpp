#include <stdlib.h> /* srand, rand */
#include <time.h> /* time */

#include "SDL.h"
#include <glm/gtx/vector_angle.hpp>

#include "AiSystem.h"

namespace {
    const float AVOID_DISTANCE = 5.f;
    const float CUTOFF_TIGHTNESS = 0.875f;
    const unsigned int MAX_COUNTER = 120;
    const int CUTOFF_COOLDOWN = 300;
    const int POWERUP_CHANCE = 240;
}

AiSystem::AiSystem()
    : num_ai_(0)
    , counter_(0)
    , whos_it_(0) {
    srand(time(NULL)); // AI uses randomness, so initialize random seed
    add_event_handler(EventType::ACTIVATE_AI, &AiSystem::handle_activate_ai, this);
    add_event_handler(EventType::OBJECT_TRANSFORM_EVENT, &AiSystem::handle_object_transform, this);
    add_event_handler(EventType::NEW_IT, &AiSystem::handle_new_it, this);
}

void AiSystem::update() {
    if (++counter_ == MAX_COUNTER) {
        counter_ = 0;
    }

    for (int i = 3; i >= 4 - num_ai_; i--) {
        perform_ai(i);

        //Randomly use powerups
        unsigned int random = rand() % POWERUP_CHANCE;

        if (random == 0) {
            EventSystem::queue_event(
                Event(
                    EventType::KEYPRESS_EVENT,
                    "player_id", i,
                    "key", random % 2 ? SDL_CONTROLLER_BUTTON_LEFTSHOULDER : SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
                    "value", SDL_CONTROLLERBUTTONDOWN
                )
            );
        }

        //We want to make sure the AI is never braking.
        EventSystem::queue_event(
            Event(
                EventType::KEYPRESS_EVENT,
                "player_id", i,
                "key", SDL_CONTROLLER_AXIS_TRIGGERLEFT,
                "value", 0
            )
        );

    }
}

void AiSystem::handle_activate_ai(const Event& e) {
    num_ai_ = e.get_value<int>("num_ai", true).first;
    std::cout << "activating " << num_ai_ << " ai" << std::endl;
}

void AiSystem::handle_object_transform(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    if (object_id > 3) {
        return;
    }

    float x = e.get_value<float>("pos_x", true).first;
    // float y = e.get_value<float>("pos_y", true).first;
    float z = e.get_value<float>("pos_z", true).first;

    float qw = e.get_value<float>("qua_w", true).first;
    float qx = e.get_value<float>("qua_x", true).first;
    float qy = e.get_value<float>("qua_y", true).first;
    float qz = e.get_value<float>("qua_z", true).first;

    cars_[object_id].position_ = glm::vec3(x, 0, z);
    cars_[object_id].rotation_ = glm::quat(qw, qx, qy, qz);
}

void AiSystem::handle_new_it(const Event& e) {
    whos_it_ = e.get_value<int>("object_id", true).first;
}

void AiSystem::perform_ai(int car) {
    if (car == whos_it_) {
        int closest_car = -1;
        float closest_car_distance = FLT_MAX;

        for (size_t i = 0; i < 4; i++) {
            if (car == i) {
                continue;
            }

            float distance = glm::distance(cars_[car].position_, cars_[i].position_);

            if (distance < closest_car_distance) {
                closest_car = i;
                closest_car_distance = distance;
            }
        }

        if (closest_car_distance < AVOID_DISTANCE) {
            avoid_car(car, closest_car);
        } else {
            random_movement(car);
        }
    } else {
        auto pos1 = cars_[car].position_;
        auto pos2 = cars_[whos_it_].position_;

        cars_[car].cutoff_cooldown_--;

        if (counter_ == 0) {
            //Check if the position of our car is approximately perpendicular to the other car relative to the arena's radius.
            if (glm::distance(pos1, pos2 * CUTOFF_TIGHTNESS) > 2.5f && cars_[car].cutoff_cooldown_ > 0) {
                cars_[car].state_ = ChaseState::CUTOFF;
                cars_[car].cutoff_cooldown_ = 300;
            }
        }

        if (glm::distance(pos1, pos2 * CUTOFF_TIGHTNESS) < 2.5f && cars_[car].state_ == ChaseState::CUTOFF) {
            cars_[car].cutoff_cooldown_ = CUTOFF_COOLDOWN;
            cars_[car].state_ = ChaseState::PURSUE;
        }

        if (cars_[car].state_ == ChaseState::CUTOFF) {
            cutoff_car(car, whos_it_);
        } else {
            pursue_car(car, whos_it_);
        }
    }
}

void AiSystem::avoid_car(int car, int to_avoid) {
    if (counter_ == 0) {
        //Make random dir -1 or 1
        cars_[car].random_dir_ = (rand() % 2) * 2 - 1;
    }

    glm::vec3 diff = cars_[to_avoid].position_ - cars_[car].position_;
    glm::vec3 target = glm::rotate(diff, glm::radians(cars_[car].random_dir_ * 90.f), glm::vec3(0, 1, 0));

    path_to(car, target);
}

void AiSystem::pursue_car(int car, int to_pursue) {
    float aheadness = glm::distance(cars_[to_pursue].position_, cars_[car].position_);
    glm::vec3 target = cars_[to_pursue].position_ + cars_[to_pursue].rotation_ * glm::vec3(0, 0, 0.5f + aheadness);

    path_to(car, target);
}

void AiSystem::random_movement(int car) {
    if (counter_ % 10 == 0) {
        unsigned int random = rand() % 20;

        if (random == 0) {
            EventSystem::queue_event(
                Event(
                    EventType::KEYPRESS_EVENT,
                    "player_id", car,
                    "key", SDL_CONTROLLER_AXIS_LEFTX,
                    "value", -32768
                )
            );
        } else if (random == 1) {
            EventSystem::queue_event(
                Event(
                    EventType::KEYPRESS_EVENT,
                    "player_id", car,
                    "key", SDL_CONTROLLER_AXIS_LEFTX,
                    "value", 32768
                )
            );
        } else {
            EventSystem::queue_event(
                Event(
                    EventType::KEYPRESS_EVENT,
                    "player_id", car,
                    "key", SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
                    "value", 32768
                )
            );
        }
    }
}

void AiSystem::cutoff_car(int car, int to_catch) {
    glm::vec3 target = cars_[to_catch].position_;
    target *= CUTOFF_TIGHTNESS;

    path_to(car, target);
}

void AiSystem::path_to(int car, const glm::vec3& point_) {

    car_data& this_car = cars_[car];

    glm::vec3 vec_to = glm::normalize(point_ - this_car.position_);
    glm::vec3 vec_facing = glm::vec3(0, 0, 1) * this_car.rotation_;

    glm::vec2 vec2_to = glm::normalize(glm::vec2(vec_to.x, vec_to.z));
    glm::vec2 vec2_facing = glm::normalize(glm::vec2(-vec_facing.x, vec_facing.z));

    float angle = glm::orientedAngle(vec2_facing, vec2_to);

    int axis = std::min(SHRT_MAX, (int)(SHRT_MAX * (abs(angle * 2.5f) - 0.05)));

    if (angle < -0.05) {
        EventSystem::queue_event(
            Event(
                EventType::KEYPRESS_EVENT,
                "player_id", car,
                "key", SDL_CONTROLLER_AXIS_LEFTX,
                "value", -axis
            )
        );
    } else if (angle > 0.05) {
        EventSystem::queue_event(
            Event(
                EventType::KEYPRESS_EVENT,
                "player_id", car,
                "key", SDL_CONTROLLER_AXIS_LEFTX,
                "value", axis
            )
        );
    } else {
        EventSystem::queue_event(
            Event(
                EventType::KEYPRESS_EVENT,
                "player_id", car,
                "key", SDL_CONTROLLER_AXIS_LEFTX,
                "value", 0
            )
        );
    }

    EventSystem::queue_event(
        Event(
            EventType::KEYPRESS_EVENT,
            "player_id", car,
            "key", SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
            "value", SHRT_MAX
        )
    );
}
