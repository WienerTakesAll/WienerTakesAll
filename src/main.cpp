#include <iostream>


#include "AssetManager.h"
#include "EventSystem.h"

#include "SDL.h"

#include "AudioSystem.h"
#include "ExampleClass.h"
#include "GameplaySystem.h"
#include "InputManager.h"
#include "RenderingSystem.h"

#include "SettingsSystem.h"

namespace {
    const std::string SETTINGS_FILE = "config/config.yml";
}

int main(int argc, char* args[]) {

    std::vector<Event> events;
    events.emplace_back(EventType::LOAD_EVENT);

    AudioSystem audio_system;
    AssetManager asset_manager;
    GameplaySystem gameplay_system;
    SettingsSystem settings_system(SETTINGS_FILE);
    InputManager input_manager(settings_system.get_input_settings());
    RenderingSystem rendering_system(asset_manager);

    if (!audio_system.init(settings_system.get_audio_settings())) {
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
        rendering_system.send_events(events);
        settings_system.send_events(events);

        gameplay_system.handle_events(events);
        rendering_system.handle_events(events);
        audio_system.handle_events(events);
        settings_system.handle_events(events);
        events.clear();


        // Gameplay
        gameplay_system.update();

        // Physics

        // Rendering
        rendering_system.update();
        rendering_system.render();

        // UI

    }

    return 0;
}
