#include <typeinfo>

#include "Event.h"

Event::Event(EventType e_type)
    : event_type(e_type) {
}

void Event::add_value(std::string name, int arg) {
    EventValue v;
    v.type_name = typeid(int).name();
    v.value.int_type = arg;
    event_values.emplace(name, v);
}

void Event::add_value(std::string name, float arg) {
    EventValue v;
    v.type_name = typeid(float).name();
    v.value.float_type = arg;
    event_values.emplace(name, v);
}

void Event::add_value(std::string name, std::string&& arg) {
    EventValue v;
    string_values.emplace(name, arg);
}

//get_value
//Params: name of event, address to store result at, bool for error checking. if true and an error occurs then exit program
template<>
bool Event::get_value(const std::string& name, std::string* value, bool crash_on_fail) const {
    const auto val = string_values.find(name); //val - pointer to the event "name"

    if (val == string_values.end()) {
        std::cout << "Value " << name << " not found in event " << static_cast<int>(event_type) << "!" << std::endl;
    
        if ( crash_on_fail == true) {   //if there is an error and crash is set to true crash program
            exit (-1);
        } else {
            return false;
        }
    }

    *value = val->second;       //update the value requested
    return true;
}
template<>
bool Event::get_value(const std::string& name, int* value, bool crash_on_fail) const {
    const std::unordered_map<std::string, EventValue>::const_iterator val = event_values.find(name);

    if (val == event_values.end()) {
        std::cout << "Value " << name << " not found in event " << static_cast<int>(event_type) << "!" << std::endl;

        if ( crash_on_fail == true) {
            exit (0);
        } else {
            return false;
        }
    }

    if (val->second.type_name != typeid(int).name()) {
        std::cout
                << "Wrong type int instead of " << val->second.type_name
                << " in value " << name << " of event " << static_cast<int>(event_type) << "!" << std::endl;

        if ( crash_on_fail == true) {
            exit (0);
        } else {
            return false;
        }
    }

    *value = val->second.value.int_type;
    return true;
}
template<>
bool Event::get_value(const std::string& name, float* value, bool crash_on_fail) const {
    const auto val = event_values.find(name);

    if (val == event_values.end()) {
        std::cout << "Value " << name << " not found in event " << static_cast<int>(event_type) << "!" << std::endl;

        if ( crash_on_fail == true) {
            exit (0);
        } else {
            return false;
        }
    }

    if (val->second.type_name != typeid(int).name()) {
        std::cout
                << "Wrong type int instead of " << val->second.type_name
                << " in value " << name << " of event " << static_cast<int>(event_type) << "!" << std::endl;

        if ( crash_on_fail == true) {
            exit (0);
        } else {
            return false;
        }
    }

    *value = val->second.value.float_type;
    return true;
}