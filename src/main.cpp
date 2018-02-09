#include <iostream>

#include "AssetManager.h"
#include "EventSystem.h"
#include "PhysicsSystem.h"

#include "SDL.h"

#include "AudioSystem.h"
#include "ExampleClass.h"
#include "GameplaySystem.h"
#include "InputManager.h"
#include "RenderingSystem.h"

int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_EVERYTHING);


    std::vector<Event> events;
    events.emplace_back(EventType::LOAD_EVENT);

    AudioSystem audio_system;
    AssetManager asset_manager;
    GameplaySystem gameplay_system;
    InputManager input_manager;
    PhysicsSystem physics_system(asset_manager);
    RenderingSystem rendering_system(asset_manager);

    if (!audio_system.init()) {
        std::cerr << "Audio system failed to initialize, continuing without audio " << std::endl;
    }

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
        input_manager.send_events(events);
        gameplay_system.send_events(events);
        physics_system.send_events(events);
        rendering_system.send_events(events);

        gameplay_system.handle_events(events);
        physics_system.handle_events(events);
        rendering_system.handle_events(events);
        audio_system.handle_events(events);
        events.clear();


        // Gameplay
        gameplay_system.update();

        // Physics
        physics_system.update();

        // Rendering
        rendering_system.update();
        rendering_system.render();

        // UI

    }

    return 0;
}
