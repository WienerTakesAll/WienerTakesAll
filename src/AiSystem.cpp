#include <stdlib.h> /* srand, rand */
#include <time.h> /* time */

#include "SDL.h"

#include "AiSystem.h"

AiSystem::AiSystem()
    : num_ai_(0)
    , counter_(0) {
    srand(time(NULL)); // AI uses randomness, so initialize random seed
    add_event_handler(EventType::ACTIVATE_AI, &AiSystem::handle_activate_ai, this);
}

void AiSystem::update() {
    // only update every 10 frames
    if (++counter_ != 10) {
        return;
    }

    counter_ = 0;

    for (int i = 0; i < num_ai_; i++) {
        unsigned int random = rand() % 20;

        if (random == 0) {
            EventSystem::queue_event(
                Event(
                    EventType::KEYPRESS_EVENT,
                    "player_id", 3 - i,
                    "key", SDL_CONTROLLER_AXIS_LEFTX,
                    "value", -32768
                )
            );
        } else if (random == 1) {
            EventSystem::queue_event(
                Event(
                    EventType::KEYPRESS_EVENT,
                    "player_id", 3 - i,
                    "key", SDL_CONTROLLER_AXIS_LEFTX,
                    "value", 32768
                )
            );
        } else {
            EventSystem::queue_event(
                Event(
                    EventType::KEYPRESS_EVENT,
                    "player_id", 3 - i,
                    "key", SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
                    "value", 32768
                )
            );
        }
    }
}

void AiSystem::handle_activate_ai(const Event& e) {
    num_ai_ = e.get_value<int>("num_ai", true).first;
    std::cout << "activating " << num_ai_ << " ai" << std::endl;
}
