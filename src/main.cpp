#include <iostream>


#include "EventSystem.h"
#include "Renderer.h"


#include "ExampleClass.h"


int main(int argc, char* args[]) {


	Renderer renderer;

	std::vector<Event> events;
	events.emplace_back(EventType::LOAD_EVENT);

	for (;;) {
		renderer.handle_events(events);
		events.clear();

		renderer.update();

		renderer.render();

		renderer.send_events(events);
	}

    return 0;
}
