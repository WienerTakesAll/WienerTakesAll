#include <iostream>

#include "EventSystem.h"
#include "InputManager.h"
#include "Renderer.h"



int main(int argc, char* args[]) {

    Renderer renderer;
    InputManager input_manager;

    std::vector<Event> events;
    events.emplace_back(EventType::LOAD_EVENT);


    // Create World

    bool game_is_running = true;

    while (game_is_running) {
        SDL_Event event;

        // Input
        while (SDL_PollEvent(&event)) {
            // Early out on quit
            if (event.type == SDL_QUIT) {
                std::cout << "SDL_QUIT was called" << std::endl;
                game_is_running = false;
                input_manager.quit();
                SDL_Quit();
                continue;
            }

            input_manager.process_input(&event);
        }

        // Events
        renderer.send_events(events);
        renderer.handle_events(events);
        events.clear();


        // Gameplay

        // Physics

        // Rendering

        renderer.update();
        renderer.render();

        // UI

    }

    return 0;
}