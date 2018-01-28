#include <iostream>
#include <chrono>


#include "AssetManager.h"
#include "EventSystem.h"

#include "SDL.h"

#include "AudioSystem.h"
#include "ExampleClass.h"

#include "InputManager.h"
#include "RenderingSystem.h"



int main(int argc, char* args[]) {

    std::vector<Event> events;
    events.emplace_back(EventType::LOAD_EVENT);

    AssetManager asset_manager;
    RenderingSystem rendering_system(asset_manager);
    InputManager input_manager;
    AudioSystem audio_system;

    if (!audio_system.init()) {
        std::cerr << "Audio system failed to initialize, continuing without audio " << std::endl;
    }

    // Create World

    bool game_is_running = true;
    using FrameDuration = std::chrono::duration<long int, std::ratio<1, 60>>;

    while (game_is_running) {
        auto start_time = std::chrono::steady_clock::now();
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
        rendering_system.send_events(events);
        input_manager.send_events(events);
        rendering_system.handle_events(events);
        audio_system.handle_events(events);
        events.clear();


        // Gameplay

        // Physics

        // Rendering

        rendering_system.update();
        rendering_system.render();

        // UI

        // Maintain a maximum frame rate of 60fps
        while ( std::chrono::duration_cast<FrameDuration>(std::chrono::steady_clock::now() - start_time).count() < 1 ) {
            // do nothing
        }
    }

    return 0;
}
