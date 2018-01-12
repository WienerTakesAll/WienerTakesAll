#include <iostream>

#include "SDL.h"

#include "EventSystem.h"
#include "InputManager.h"

SDL_Window* window = NULL;


class ExampleClass : public EventSystem<ExampleClass> {
public:
    //Pretend this does something
    void handleExampleEvent(Event some_event);

    //Here's what a constructor should look like
    ExampleClass() {
        //This is how to create a handler for an event.
        //"someKindofEvent" is the name of the event
        //"&ExampleClass::handleExampleEvent" is a function that will be called on the event.
        //--> Note that the handler MUST have type void(Event) <--
        //"this" is the pointer of this class. A little redundant, but it's the best I could do.
        EventSystem::addEventHandler("someKindofEvent", &ExampleClass::handleExampleEvent, this);
    }


    //Lets use an example function
    void doSomething() {
        //...
        //Blah blah blah blah
        //...

        //Now let's say we want to send an event...
        EventSystem::queueEvent(
            Event("someKindofEvent",
                  "I am a string type", "I am a value of a string type",
                  "I am an int type", 5,
                  "I am a float type", 5.0f,
                  "I am a string type again?", "It seems so... and etc"
                 )
        );
        //Now the event is stored internally, but we need to push it to a list somewhere down the line...
    }

};

void ExampleClass::handleExampleEvent(Event some_event) {
    //Lets grab what values we might need

    //neededString will be the string, or "error" if there is no such string
    std::string neededString = some_event.getValue<std::string>("I am a string type", "error");

    //neededInt will be the int, or -1 if the value is either not there or not an int
    int neededInt = some_event.getValue<int>("I am an int type", -1);

    //Same idea
    float neededFloat = some_event.getValue<float>("I am a float type", -1.0f);

    //...
    //Now we could do something based on these values.
    std::cout << "String value: " << neededString << std::endl;
    std::cout << "int value: " << neededInt << std::endl;
    std::cout << "float value: " << neededFloat << std::endl;
}


int main(int argc, char* args[]) {

    ExampleClass example;
    example.doSomething();
    std::vector<Event> allEvents;
    example.sendEvents(allEvents);
    example.handleEvents(allEvents);



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

    InputManager input_manager;

    bool game_is_running = true;

    while (game_is_running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            // Early out on quit
            if (event.type == SDL_QUIT) {
                std::cout << "SDL_QUIT was called" << std::endl;
                SDL_Quit();
                game_is_running = false;
            }

            input_manager.process_input(&event);
        }
    }

    return 0;
}
