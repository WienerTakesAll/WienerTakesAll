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

void Event::add_value(std::string name, void* arg) {
    EventValue v;
    v.type_name = typeid(void*).name();
    v.value.pointer_type = arg;
    event_values_.emplace(name, v);
}

template<>
int Event::get_value(const std::string& name, int otherwise) const {
    const auto val = event_values_.find(name);

    if (val == event_values_.end()) {
        std::cout << "Value " << name << " not found in event " << static_cast<int>(event_type) << "!" << std::endl;
        return otherwise;
    }

    if (val->second.type_name != typeid(int).name()) {
        std::cout
                << "Wrong type int instead of " << val->second.type_name
                << " in value " << name << " of event " << static_cast<int>(event_type) << "!" << std::endl;
        return otherwise;
    }

    return val->second.value.int_type;
}

template<>
float Event::get_value(const std::string& name, float otherwise) const {
    const auto val = event_values_.find(name);

    if (val == event_values_.end()) {
        std::cout << "Value " << name << " not found in event " << static_cast<int>(event_type) << "!" << std::endl;
        return otherwise;
    }

    if (val->second.type_name != typeid(float).name()) {
        std::cout
                << "Wrong type float instead of " << val->second.type_name
                << " in value " << name << " of event " << static_cast<int>(event_type) << "!" << std::endl;
        return otherwise;
    }

    return val->second.value.float_type;
}

template<>
std::string Event::get_value(const std::string& name, std::string otherwise) const {
    const auto val = string_values_.find(name);

    if (val == string_values_.end()) {
        std::cout << "String Value " << name << " not found in event " << static_cast<int>(event_type) << "!" << std::endl;
        return otherwise;
    }

    return val->second;
}

template<>
void* Event::get_value(const std::string& name, void* otherwise) const {
    const auto val = event_values_.find(name);

    if (val == event_values_.end()) {
        std::cerr << "Value " << name << " not found in event" << static_cast<int>(event_type) << "!" << std::endl;
        return otherwise;
    }

    if (val->second.type_name != typeid(void*).name()) {
        std::cerr
                << "Wrong type void* instead of " << val->second.type_name
                << " in value " << name << " of event " << static_cast<int>(event_type) << "!" << std::endl;
        return otherwise;
    }

    return val->second.value.pointer_type;
}
