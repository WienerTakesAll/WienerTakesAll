#include <iostream>


#include "AssetManager.h"
#include "EventSystem.h"

#include "SDL.h"

#include "AudioSystem.h"
#include "ExampleClass.h"

#include "InputManager.h"
#include "Renderer.h"



int main(int argc, char* args[]) {

    std::vector<Event> events;
    events.emplace_back(EventType::LOAD_EVENT);

    AssetManager asset_manager;
    Renderer renderer(asset_manager);
    InputManager input_manager;
    AudioSystem audio_system;

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
                audio_system.quit();
                input_manager.quit();
                SDL_Quit();
                continue;
            }

            input_manager.process_input(&event);
        }

        // Events
        renderer.send_events(events);
        input_manager.send_events(events);
        renderer.handle_events(events);
        audio_system.handle_events(events);
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