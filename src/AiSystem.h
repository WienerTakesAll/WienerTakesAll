#pragma once

#include "EventSystem.h"

class AiSystem : public EventSystem<AiSystem> {
public:
	AiSystem();
	void update();

private:
	void handle_activate_ai(const Event& e);
	int num_ai_;
};