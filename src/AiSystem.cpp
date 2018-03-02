#include "SDL.h"

#include "AiSystem.h"

AiSystem::AiSystem()
: num_ai_(0) {
    add_event_handler(EventType::ACTIVATE_AI, &AiSystem::handle_activate_ai, this);
}

void AiSystem::update() {
	for(int i = 0; i < num_ai_; i++) {
        std::cout << "sending for ai " << 3 -i << std::endl;
        EventSystem::queue_event(
            Event(
                EventType::KEYPRESS_EVENT,
                "player_id", 3 - i,
                "key", SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
                "value", 1
            )
        );
	}
}

void AiSystem::handle_activate_ai(const Event& e) {
    num_ai_ = e.get_value<int>("num_ai", true).first;
    std::cout << "activating " << num_ai_ << " ai" << std::endl;
}
