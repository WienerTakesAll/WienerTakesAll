#include <stdlib.h> /* srand, rand */
#include <time.h> /* time */

#include "SDL.h"
#include <glm/gtx/vector_angle.hpp>

#include "AiSystem.h"

AiSystem::AiSystem()
    : num_ai_(0)
    , counter_(0)
    , whos_it_(0) {
    srand(time(NULL)); // AI uses randomness, so initialize random seed
    add_event_handler(EventType::ACTIVATE_AI, &AiSystem::handle_activate_ai, this);
    add_event_handler(EventType::OBJECT_TRANSFORM_EVENT, &AiSystem::handle_object_transform, this);
}

void AiSystem::update() {
    // only update every 10 frames
    if (++counter_ == 10) {
        counter_ = 0;
    }


	for (int i = 3; i >= 4 - num_ai_; i--) {
		if (i != whos_it_)
		{
			path_to(i, cars_[whos_it_].position_);
		}
		else if(counter_ == 0)
		{
			unsigned int random = rand() % 20;

			if (random == 0) {
				EventSystem::queue_event(
					Event(
						EventType::KEYPRESS_EVENT,
						"player_id", i,
						"key", SDL_CONTROLLER_AXIS_LEFTX,
						"value", -32768
					)
				);
			}
			else if (random == 1) {
				EventSystem::queue_event(
					Event(
						EventType::KEYPRESS_EVENT,
						"player_id", i,
						"key", SDL_CONTROLLER_AXIS_LEFTX,
						"value", 32768
					)
				);
			}
			else {
				EventSystem::queue_event(
					Event(
						EventType::KEYPRESS_EVENT,
						"player_id", i,
						"key", SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
						"value", rand() % 32768
					)
				);
			}
		}


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
    float y = e.get_value<float>("pos_y", true).first;
    float z = e.get_value<float>("pos_z", true).first;

    float qw = e.get_value<float>("qua_w", true).first;
    float qx = e.get_value<float>("qua_x", true).first;
    float qy = e.get_value<float>("qua_y", true).first;
    float qz = e.get_value<float>("qua_z", true).first;

	cars_[object_id].position_ = glm::vec3(x, 0, z);
	cars_[object_id].rotation_ = glm::quat(qw, qx, qy, qz);
}


void AiSystem::path_to(int car, const glm::vec3& point_) {

	car_data& this_car = cars_[car];

	glm::vec3 vec_to = glm::normalize(point_ - this_car.position_);
	glm::vec3 vec_facing = glm::vec3(0,0,1) * this_car.rotation_;

	glm::vec2 vec2_to = glm::normalize(glm::vec2(vec_to.x, vec_to.z));
	glm::vec2 vec2_facing = glm::normalize(glm::vec2(-vec_facing.x, vec_facing.z));

	float angle = glm::orientedAngle(vec2_facing, vec2_to);

	int axis = std::min(SHRT_MAX, (int)(SHRT_MAX * (abs(angle)-0.05)));

	if (angle < -0.05) {
		EventSystem::queue_event(
			Event(
				EventType::KEYPRESS_EVENT,
				"player_id", car,
				"key", SDL_CONTROLLER_AXIS_LEFTX,
				"value", -axis
			)
		);
	}
	else if (angle > 0.05) {
		EventSystem::queue_event(
			Event(
				EventType::KEYPRESS_EVENT,
				"player_id", car,
				"key", SDL_CONTROLLER_AXIS_LEFTX,
				"value", axis
			)
		);
	}
	else {
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