#include <iostream>

#include "SDL.h"

#include "AudioSystem.h"
#include "ExampleClass.h"
#include "InputManager.h"

SDL_Window* window = NULL;


int main(int argc, char* args[]) {

    ExampleClass example;
    example.do_something();
    std::vector<Event> allEvents;
    example.send_events(allEvents);
    example.handle_events(allEvents);



    SDL_Init(SDL_INIT_EVERYTHING);

    int sdl_flags = SDL_WINDOW_SHOWN;

    int screen_width = 640;
    int screen_height = 480;

    window = SDL_CreateWindow("WienerTakesAll",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              screen_width,
                              screen_height,
                              sdl_flags);

    if (window == NULL) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "Driver: " <<  SDL_GetCurrentVideoDriver() << std::endl;

    AudioSystem audio_system;
    InputManager input_manager;

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
        input_manager.send_events(allEvents);
        audio_system.handle_events(allEvents);
        allEvents.clear();

        // Gameplay

        // Physics

        // Rendering

        // UI

    }

    return 0;
}
