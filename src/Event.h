#pragma once

#include <iostream>
#include <unordered_map>
#include <string>

//For how to handle events, please view EventSystem.h
struct Event {
    Event(std::string&& name);
    //Construct an event from a variable number of arguments
    template <typename... T>
    Event(std::string&& name, T... args);

    std::string eventName;
    struct EventValue {
        std::string typeName;
        union ValueType {
            int         int_type;
            float       float_type;
            //void*       pointer_type; Perhaps?
        };
        ValueType value;
    };


    std::unordered_map<std::string, EventValue> eventValues;
    //Due to strings having non-trivial destructors, I can't put them in the union! Oops!
    std::unordered_map<std::string, std::string> stringValues;

    void add_value(std::string name, int arg);
    void add_value(std::string name, float arg);
    void add_value(std::string name, std::string&& arg);

    template<typename T>
    T getValue(const std::string& name, T otherwise);

private:

    template <typename U, typename... T>
    void event_impl(std::string&& name, U&& arg, T... rest);
    template <typename U>
    void event_impl(std::string&& name, U&& arg);


};

#include "Event.impl"
