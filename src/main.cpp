#include <iostream>

#include "EventSystem.h"
#include "Renderer.h"


int main(int argc, char* args[]) {

	Renderer renderer;

	std::vector<Event> events;
	events.emplace_back("load");

	for (;;) {
		renderer.handleEvents(events);
		events.clear();

		renderer.update();

		renderer.render();

		renderer.sendEvents(events);
	}

	return 0;
}
