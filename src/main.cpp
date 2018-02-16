#include <cassert>
#include <iostream>

#include <chrono>
#include <thread>


#include "AssetManager.h"
#include "EventSystem.h"
#include "PhysicsSystem.h"

#include "SDL.h"
#include "SDL_image.h"

#include "AudioSystem.h"
#include "ExampleClass.h"
#include "GameplaySystem.h"
#include "InputManager.h"
#include "RenderingSystem.h"
#include "UISystem.h"


namespace {
    // Length of one frame (60fps ~= 16.66 milliseconds per frame)
    const auto FRAME_DURATION_MS = std::chrono::milliseconds( 16 );
}

int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_EVERYTHING);

    std::vector<Event> events;
    events.emplace_back(EventType::LOAD_EVENT);


    AssetManager asset_manager;
    GameplaySystem gameplay_system;
    InputManager input_manager;
    AudioSystem audio_system;
    UISystem ui_system(asset_manager);
    PhysicsSystem physics_system;
    RenderingSystem rendering_system(asset_manager);


    if (!audio_system.init()) {
        std::cerr << "Audio system failed to initialize, continuing without audio " << std::endl;
    }

    // Create World

    bool game_is_running = true;

    while (game_is_running) {
        auto frame_end_time = std::chrono::steady_clock::now() + FRAME_DURATION_MS;
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
        ui_system.send_events(events);
        gameplay_system.send_events(events);
        rendering_system.send_events(events);

        input_manager.handle_events(events);
        gameplay_system.handle_events(events);
        rendering_system.handle_events(events);
        audio_system.handle_events(events);
        ui_system.handle_events(events);
        events.clear();


        // Gameplay
        gameplay_system.update();

        // Physics

        // Rendering
        rendering_system.update();
        rendering_system.render();

        // UI
        ui_system.update();
        ui_system.render();

        // Maintain a maximum frame rate of 60fps
        if ( game_is_running ) {
            std::this_thread::sleep_until( frame_end_time );
        }
    }

    return 0;
}
