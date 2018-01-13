#include <iostream>

#include "SDL.h"

#include "ExampleClass.h"

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

    for (;;) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    std::cout << "SDL_QUIT was called" << std::endl;
                    SDL_Quit();
                    break;
            }
        }
    }

    return 0;
}
