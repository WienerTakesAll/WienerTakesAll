#pragma once

#include "EventSystem.h"

class ExampleClass : public EventSystem<ExampleClass> {
public:
    //Pretend this does something
    void handleExampleEvent(const Event& some_event);

    //Here's what a constructor should look like
    ExampleClass() {
        //This is how to create a handler for an event.
        //"someKindofEvent" is the name of the event
        //"&ExampleClass::handleExampleEvent" is a function that will be called on the event.
        //--> Note that the handler MUST have type void(Event) <--
        //"this" is the pointer of this class. A little redundant, but it's the best I could do.
        EventSystem::addEventHandler(EventType::load, &ExampleClass::handleExampleEvent, this);
    }


    //Lets use an example function
    void doSomething() {
        //...
        //Blah blah blah blah
        //...

        //Now let's say we want to send an event...
        EventSystem::queueEvent(
            Event(EventType::load,
                  "I am a string type", "I am a value of a string type",
                  "I am an int type", 5,
                  "I am a float type", 5.0f,
                  "I am a string type again?", "It seems so... and etc"
                 )
        );
        //Now the event is stored internally, but we need to push it to a list somewhere down the line...
    }

};

void ExampleClass::handleExampleEvent(const Event& some_event) {
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
