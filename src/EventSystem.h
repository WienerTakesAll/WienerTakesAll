#pragma once

#include <string>
#include <functional>
#include <vector>
#include <unordered_map>

#include "Event.h"

template<class T>
class EventSystem
{
public:
	EventSystem() = default;
	~EventSystem() = default;

	//Add a function to handle an event of name eventName. Also requires "this" pointer of object.
	//Example Usage: addEventHandler("someEvent", &ExampleClass::ExampleFunction, this)
	void addEventHandler(std::string&& eventName, std::function<void(T*,Event)> handleFunction, T* this_pointer);

	//Takes a list of events and runs the specified function handlers
	void handleEvents(const std::vector<Event>& events);

	//Construct and queue an event.
	//Example Usage: queueEvent("someEvent", "int_value", 10, "float_value", 5.5f, "string_value", "text")
	void queueEvent(Event&& queuedEvent);

	//Send the accumulated events to a list.
	//This should be used to congregate the events of all the systems before handling
	void sendEvents(std::vector<Event>& reciever);

private:

	std::vector<Event> eventQueue;
	std::unordered_map<std::string, std::function<void(Event)>> functionHandlers;
};
#include "EventSystem.impl"

/*

	--How to use the event system--

	A class must inheret from EventSystem and will then be able to use it.
	Below I have given an example of what a class that uses it may look like.



//Let's create a class that can handle a "someKindofEvent" event
class ExampleClass : public EventSystem<ExampleClass>
{
public:
	//Pretend this does something
	void handleExampleEvent(Event some_event);

	//Here's what a constructor should look like
	ExampleClass()
	{
			//This is how to create a handler for an event.
			//"someKindofEvent" is the name of the event
			//"&ExampleClass::handleExampleEvent" is a function that will be called on the event.
			//--> Note that the handler MUST have type void(Event) <--
			//"this" is the pointer of this class. A little redundant, but it's the best I could do.
		EventSystem::addEventHandler("someKindofEvent", &ExampleClass::handleExampleEvent, this);
	}


	//Lets use an example function
	void doSomething()
	{
		//...
		//Blah blah blah blah
		//...

		//Now let's say we want to send an event...
		EventSystem::queueEvent(
			Event("someKindofEvent",
				"I am a string type", "I am a value of a string type",
				"I am an int type", 5,
				"I am a float type", 5.0f,
				"I am a string type again?", "It seems so... and etc"
			)
		);
		//Now the event is stored internally, but we need to push it to a list somewhere down the line...
	}

};

//So how might we handle a specific event?
void ExampleClass::handleExampleEvent(Event some_event)
{
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

If you want to see this in action, run the following lines of code:

	ExampleClass example;
	example.doSomething();
	std::vector<Event> allEvents;
	example.sendEvents(allEvents);
	example.handleEvents(allEvents);

This should print the values that were sent and then recieved.
*/
