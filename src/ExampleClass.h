#pragma once

#include "EventSystem.h"

class ExampleClass : public EventSystem<ExampleClass> {
public:
    //Pretend this does something
    void handle_example_event(const Event& some_event);

    //Here's what a constructor should look like
    ExampleClass() {
        //This is how to create a handler for an event.
        //"someKindofEvent" is the name of the event
        //"&ExampleClass::handleExampleEvent" is a function that will be called on the event.
        //--> Note that the handler MUST have type void(Event) <--
        //"this" is the pointer of this class. A little redundant, but it's the best I could do.
        EventSystem::add_event_handler(EventType::DUMMY_EVENT, &ExampleClass::handle_example_event, this);
    }


    //Lets use an example function
    void do_something() {
        //...
        //Blah blah blah blah
        //...

        //Now let's say we want to send an event...
        EventSystem::queue_event(
            Event(EventType::DUMMY_EVENT,
                  "I am a string type", "I am a value of a string type",
                  "I am an int type", 5,
                  "I am a float type", 5.0f,
                  "I am a string type again?", "It seems so... and etc"
                 )
        );
        //Now the event is stored internally, but we need to push it to a list somewhere down the line...
    }

};

void ExampleClass::handle_example_event(const Event& some_event) {
    //Lets grab what values we might need

    //neededString will be the string
    std::string needed_string;
    bool has_string = some_event.get_value<std::string>("I am a string type", &needed_string, false); // name of the type, address to store at, crash_on_fail bool

    //neededInt will be the int
    int needed_int;
    bool has_int = some_event.get_value<int>("I am an int type", &needed_int, false); // name of the type, address to store at, crash_on_fail bool

    //Same idea
    float needed_float;
    bool has_float = some_event.get_value<float>("I am a float type", &needed_float, false);

    //...
    //Now we could do something based on these values.
    std::cout << "String value: " << needed_string << std::endl;
    std::cout << "int value: " << needed_int << std::endl;
    std::cout << "float value: " << needed_float << std::endl;
}
