#pragma once

#include <iostream>
#include <unordered_map>
#include <string>

enum class EventType {
    ADD_CAR,
    ADD_ARENA,
    LOAD_EVENT,
    DUMMY_EVENT,
    OBJECT_TRANSFORM_EVENT,
    KEYPRESS_EVENT,

    // Car control events
    CAR_CONTROL,

    // Keep at end of list
    EVENT_COUNT
};

//For how to handle events, please view EventSystem.h
class Event {
public:
    Event(EventType e_type);
    //Construct an event from a variable number of arguments
    template <typename... T>
    Event(EventType eType, T... args);

    EventType event_type;
    struct EventValue {
        std::string type_name;
        union ValueType {
            int         int_type;
            float       float_type;
            bool        bool_type;
            //void*       pointer_type; Perhaps?
        };
        ValueType value;
    };

    void add_value(std::string name, int arg);
    void add_value(std::string name, float arg);
    void add_value(std::string name, std::string&& arg);

    template<typename T>
    std::pair<T, bool> get_value(const std::string& name, bool crash_on_fail) const;

private:

    std::unordered_map<std::string, EventValue> event_values_;
    //Due to strings having non-trivial destructors, I can't put them in the union! Oops!
    std::unordered_map<std::string, std::string> string_values_;

    template <typename U, typename... T>
    void event_impl(std::string&& name, U&& arg, T... rest);
    template <typename U>
    void event_impl(std::string&& name, U&& arg);
};

#include "Event_impl.h"
