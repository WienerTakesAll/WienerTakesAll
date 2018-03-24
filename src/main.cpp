#define PROFILING 0

#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <stdio.h>

#include "AssetManager.h"
#include "EventSystem.h"
#include "PhysicsSystem.h"

#include "SDL.h"
#include "SDL_image.h"

#include "AssetManager.h"
#include "AudioSystem.h"
#include "EventSystem.h"
#include "ExampleClass.h"
#include "GameplaySystem.h"
#include "InputManager.h"
#include "PhysicsSystem.h"
#include "PhysicsSettings.h"
#include "RenderingSystem.h"
#include "UISystem.h"
#include "AiSystem.h"

#include "SettingsSystem.h"

namespace {
    const std::string SETTINGS_FILE = "config/config.yml";

    // Length of one frame (60fps ~= 16.66 milliseconds per frame)
    const auto FRAME_DURATION_MS = std::chrono::milliseconds( 16 );

    template <bool Profile, typename C>
    void runProfiledUpdate(C& runner, std::chrono::duration<double>& time)
    {
        if (Profile)
        {
            auto func_start = std::chrono::steady_clock::now();
            runner.update();
            time = std::chrono::steady_clock::now() - func_start;
        }
        else
        {
            runner.update();
        }
    }

    template <bool Profile, typename C>
    void runProfiledRender(C& runner, std::chrono::duration<double>& time)
    {
        if (Profile)
        {
            auto func_start = std::chrono::steady_clock::now();
            runner.render();
            time = std::chrono::steady_clock::now() - func_start;
        }
        else
        {
            runner.render();
        }
    }
}




int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_EVERYTHING);

    std::vector<Event> events;
    events.emplace_back(EventType::LOAD_EVENT);


    SettingsSystem settings_system(SETTINGS_FILE);


    AudioSystem audio_system(settings_system.get_audio_settings());
    AssetManager asset_manager;
    RenderingSystem rendering_system(asset_manager);
    GameplaySystem gameplay_system;
    InputManager input_manager(settings_system.get_input_settings());
    UISystem ui_system(asset_manager);
    PhysicsSystem physics_system(asset_manager, settings_system.get_physics_settings());
    AiSystem ai_system;

    if (!audio_system.init()) {
        std::cerr << "Audio system failed to initialize, continuing without audio " << std::endl;
    }

    // Create World

    bool game_is_running = true;

    while (game_is_running) {
        auto frame_start_time = std::chrono::steady_clock::now();
        auto frame_end_time = frame_start_time + FRAME_DURATION_MS;
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


        auto events_start = std::chrono::steady_clock::now();
        // Send Events
        input_manager.send_events(events);
        gameplay_system.send_events(events);
        physics_system.send_events(events);
        rendering_system.send_events(events);
        settings_system.send_events(events);
        ui_system.send_events(events);
        ai_system.send_events(events);

        // Handle events
        input_manager.handle_events(events);
        gameplay_system.handle_events(events);
        physics_system.handle_events(events);
        rendering_system.handle_events(events);
        ui_system.handle_events(events);
        audio_system.handle_events(events);
        settings_system.handle_events(events);
        ai_system.handle_events(events);

        // Clear events
        events.clear();
        std::chrono::duration<double> events_elapsed = std::chrono::steady_clock::now() - events_start;

        // Update
        std::chrono::duration<double> ai_elapsed_update;
        runProfiledUpdate<PROFILING>(ai_system, ai_elapsed_update);

        std::chrono::duration<double> gameplay_elapsed_update;
        runProfiledUpdate<PROFILING>(gameplay_system, gameplay_elapsed_update);

        std::chrono::duration<double> physics_elapsed_update;
        runProfiledUpdate<PROFILING>(physics_system, physics_elapsed_update);

        std::chrono::duration<double> rendering_elapsed_update;
        runProfiledUpdate<PROFILING>(rendering_system, rendering_elapsed_update);

        std::chrono::duration<double> ui_elapsed_update;
        runProfiledUpdate<PROFILING>(ui_system, ui_elapsed_update);


        // Render
        std::chrono::duration<double> rendering_elapsed_render;
        runProfiledRender<PROFILING>(rendering_system, rendering_elapsed_render);

        std::chrono::duration<double> ui_elapsed_render;
        runProfiledRender<PROFILING>(ui_system, ui_elapsed_render);

        // Maintain a maximum frame rate of 60fps
        if ( game_is_running ) {
            std::chrono::duration<double> diff =
                std::chrono::steady_clock::now() - frame_end_time;

            if (PROFILING) {
                printf("E:%f%%, AU:%f%%, GU:%f%%, PU:%f%%, RU:%f%%, UU:%f%%, RR:%f%%, UR:%f%%, I:%f%%\n",
                       events_elapsed.count() * 60.f * 100,
                       ai_elapsed_update.count() * 60.f * 100,
                       gameplay_elapsed_update.count() * 60.f * 100,
                       physics_elapsed_update.count() * 60.f * 100,
                       rendering_elapsed_update.count() * 60.f * 100,
                       ui_elapsed_update.count() * 60.f * 100,
                       rendering_elapsed_render.count() * 60.f * 100,
                       ui_elapsed_render.count() * 60.f * 100,
                       -diff.count() * 60.f * 100);
            }

            std::this_thread::sleep_until( frame_end_time );
        }

    }

    return 0;
}
