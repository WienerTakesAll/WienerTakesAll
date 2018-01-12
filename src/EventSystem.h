#pragma once

#include <string>
#include <functional>
#include <vector>
#include <array>

#include "Event.h"

template<class T>
class EventSystem {
public:
    EventSystem() = default;
    ~EventSystem() = default;

    //Add a function to handle an event of name eventName. Also requires "this" pointer of object.
    //Example Usage: add_event_handler("someEvent", &ExampleClass::ExampleFunction, this)
    void add_event_handler(EventType event_type, std::function<void(T*, Event)> handle_function, T* this_pointer);

    //Takes a list of events and runs the specified function handlers
    void handle_events(const std::vector<Event>& events);

    //Construct and queue an event.
    //Example Usage: queue_event("someEvent", "int_value", 10, "float_value", 5.5f, "string_value", "text")
    void queue_event(Event&& queuedEvent);

    //Send the accumulated events to a list.
    //This should be used to congregate the events of all the systems before handling
    void send_events(std::vector<Event>& reciever);

private:

    std::vector<Event> event_queue;
    std::array<std::function<void(const Event&)>, static_cast<int>(EventType::EVENT_COUNT)> function_handlers;
};
#include "EventSystem_impl.h"