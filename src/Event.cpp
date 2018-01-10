#include "Event.h"

Event::Event(std::string&& name)
	: eventName(name)
{
}

void Event::add_value(std::string name, int arg)
{
	EventValue v;
	v.typeName = typeid(int).name();
	v.value.int_type = arg;
	eventValues.emplace(name, v);
}

void Event::add_value(std::string name, float arg)
{
	EventValue v;
	v.typeName = typeid(float).name();
	v.value.float_type = arg;
	eventValues.emplace(name, v);
}

void Event::add_value(std::string name, std::string&& arg)
{
	EventValue v;
	stringValues.emplace(name, arg);
}

template<>
int Event::getValue(const std::string& name, int otherwise)
{
	const auto val = eventValues.find(name);

	if (val == eventValues.end())
	{
		std::cout << "Value " << name << " not found in event " << eventName << "!" << std::endl;
		return otherwise;
	}

	if (val->second.typeName != typeid(int).name())
	{
		std::cout
			<< "Wrong type int instead of " << val->second.typeName
			<< " in value " << name << " of event " << eventName << "!" << std::endl;
		return otherwise;
	}

	return val->second.value.int_type;
}

template<>
float Event::getValue(const std::string& name, float otherwise)
{
	const auto val = eventValues.find(name);

	if (val == eventValues.end())
	{
		std::cout << "Value " << name << " not found in event " << eventName << "!" << std::endl;
		return otherwise;
	}

	if (val->second.typeName != typeid(float).name())
	{
		std::cout
			<< "Wrong type float instead of " << val->second.typeName
			<< " in value " << name << " of event " << eventName << "!" << std::endl;
		return otherwise;
	}

	return val->second.value.float_type;
}

template<>
std::string Event::getValue(const std::string& name, std::string otherwise)
{
	const auto val = stringValues.find(name);

	if (val == stringValues.end())
	{
		std::cout << "String Value " << name << " not found in event " << eventName << "!" << std::endl;
		return otherwise;
	}

	return val->second;
}