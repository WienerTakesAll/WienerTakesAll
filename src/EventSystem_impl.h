#pragma once
template<typename T>
void EventSystem<T>::addEventHandler(std::string&& eventName, std::function<void(T*, Event)> handleFunction, T* this_pointer) {
    functionHandlers.emplace(eventName, std::bind(handleFunction, this_pointer, std::placeholders::_1));
}

template<typename T>
void EventSystem<T>::handleEvents(const std::vector<Event>& events) {
    for (const auto& some_event : events) {
        const auto handler = functionHandlers.find(some_event.eventName);

        //If there is a registered function to handle the event then we do.
        if (handler != functionHandlers.end()) {
            handler->second(some_event);
        }
    }
}

template<class T>
void EventSystem<T>::queueEvent(Event&& queuedEvent) {
    eventQueue.emplace_back(queuedEvent);
}

template<class T>
void EventSystem<T>::sendEvents(std::vector<Event>& reciever) {
    reciever.insert(reciever.end(),
                    std::make_move_iterator(eventQueue.begin()),
                    std::make_move_iterator(eventQueue.end()));
    eventQueue.clear();
}
