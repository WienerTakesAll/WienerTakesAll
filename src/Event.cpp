#include <typeinfo>

#include "Event.h"

Event::Event(EventType e_type)
    : event_type(e_type) {
}

void Event::add_value(std::string name, int arg) {
    EventValue v;
    v.type_name = typeid(int).name();
    v.value.int_type = arg;
    event_values_.emplace(name, v);
}

void Event::add_value(std::string name, float arg) {
    EventValue v;
    v.type_name = typeid(float).name();
    v.value.float_type = arg;
    event_values_.emplace(name, v);
}

void Event::add_value(std::string name, std::string&& arg) {
    EventValue v;
    string_values_.emplace(name, arg);
}

//get_value
//Params: name of event, address to store result at, bool for error checking. if true and an error occurs then exit program
template<>
std::pair<std::string, bool> Event::get_value(const std::string& name, bool crash_on_fail) const {
    const auto val = string_values_.find(name); //val - pointer to the event "name"

    if (val == string_values_.end()) {
        std::cout << "Value " << name << " not found in event " << static_cast<int>(event_type) << "!" << std::endl;

        if (crash_on_fail == true) {   //if there is an error and crash is set to true crash program
            std::cerr << "Error: Value not found and program will crash on fail.";
            exit(EXIT_FAILURE);
        } else {
            return std::make_pair("-1", false);
        }
    }

    return std::make_pair(val->second, true);
}
template<>
std::pair<int, bool> Event::get_value(const std::string& name, bool crash_on_fail) const {
    const auto val = event_values_.find(name);

    if (val == event_values_.end()) {
        std::cout << "Value " << name << " not found in event " << static_cast<int>(event_type) << "!" << std::endl;

        if (crash_on_fail == true) {
            std::cerr << "Error: Value not found and program will crash on fail.";
            exit(EXIT_FAILURE);
        } else {
            return std::make_pair(-1, false);
        }
    }

    if (val->second.type_name != typeid(int).name()) {
        std::cout
                << "Wrong type int instead of " << val->second.type_name
                << " in value " << name << " of event " << static_cast<int>(event_type) << "!" << std::endl;

        if (crash_on_fail == true) {
            std::cerr << "Error: Wrong type int and program will crash on fail.";
            exit(EXIT_FAILURE);
        } else {
            return std::make_pair(-1, false);
        }
    }

    return std::make_pair(val->second.value.int_type, true);
}
template<>
std::pair<float, bool> Event::get_value(const std::string& name, bool crash_on_fail) const {
    const auto val = event_values_.find(name);

    if (val == event_values_.end()) {
        std::cout << "Value " << name << " not found in event " << static_cast<int>(event_type) << "!" << std::endl;

        if (crash_on_fail == true) {
            std::cerr << "Error: Value not found and program will crash on fail.";
            exit(EXIT_FAILURE);
        } else {
            return std::make_pair(-1.0, false);
        }
    }

    if (val->second.type_name != typeid(float).name()) {
        std::cout
                << "Wrong type int instead of " << val->second.type_name
                << " in value " << name << " of event " << static_cast<int>(event_type) << "!" << std::endl;

        if (crash_on_fail == true) {
            std::cerr << "Error: Wrong type and program will crash on fail.";
            exit(EXIT_FAILURE);
        } else {
            return std::make_pair(-1.0, false);
        }
    }

    return std::make_pair(val->second.value.float_type, true);
}