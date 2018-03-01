#pragma once

template<typename T>
void EventSystem<T>::add_event_handler(EventType eventType, std::function<void(T*, Event)> handle_function, T* this_pointer) {
    function_handlers_[static_cast<int>(eventType)] = std::bind(handle_function, this_pointer, std::placeholders::_1);
}

template<typename T>
void EventSystem<T>::handle_events(const std::vector<Event>& events) {
    for (const auto& some_event : events) {
        const auto& handler = function_handlers_[static_cast<int>(some_event.event_type)];

        //If there is a registered function to handle the event then we do.
        if (handler != nullptr) {
            handler(some_event);
        }
    }
}

template<class T>
void EventSystem<T>::queue_event(Event&& queuedEvent) {
    event_queue_.emplace_back(queuedEvent);
}

template<class T>
void EventSystem<T>::send_events(std::vector<Event>& reciever) {
    reciever.insert(reciever.end(),
                    std::make_move_iterator(event_queue_.begin()),
                    std::make_move_iterator(event_queue_.end()));
    event_queue_.clear();
}
