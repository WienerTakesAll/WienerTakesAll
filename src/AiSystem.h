#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "EventSystem.h"

class AiSystem : public EventSystem<AiSystem> {
public:
    AiSystem();
    void update();

private:
    void handle_activate_ai(const Event& e);
    void handle_object_transform(const Event& e);
    void handle_new_it(const Event& e);

    void perform_ai(int car);
    void avoid_car(int car, int to_avoid);
    void pursue_car(int car, int to_pursue);
    void random_movement(int car);
    void cutoff_car(int car, int to_catch);
    void path_to(int car, const glm::vec3& point_);

    int num_ai_;

    enum class ChaseState {PURSUE, CUTOFF};

    struct car_data {
        glm::vec3 position_;
        glm::quat rotation_;
        ChaseState state_;
        int cutoff_cooldown_;
    };

    std::array<car_data, 4> cars_;
    unsigned int counter_;
    unsigned int whos_it_;
};
