#pragma once

template <typename U>
void Event::event_impl(std::string&& name, U&& arg)
{
	add_value(name, arg);
}

template <typename U, typename... T>
void Event::event_impl(std::string&& name, U&& arg, T... rest)
{
	add_value(name, arg);
	event_impl(rest...);
}



template <typename... T>
Event::Event(EventType eType, T... args)
	: eventType(eType)
{
	event_impl(args...);
}
