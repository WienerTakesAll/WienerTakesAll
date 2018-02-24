#include "GameObjectCounter.h"

int GameObjectCounter::id_ = -1;
bool GameObjectCounter::instance_flag_ = false;
GameObjectCounter* GameObjectCounter::counter_ = nullptr;

// Returns a pointer to the current instance of GameObjectCounter
GameObjectCounter* GameObjectCounter::get_instance() {
    if (!instance_flag_) {
        id_ = 0;
        instance_flag_ = true;
        counter_ = new GameObjectCounter();
    }

    return counter_;
}

// Returns the next available object id
int GameObjectCounter::assign_id() const {
    return id_++;
}

// GameObjectCounter destructor
GameObjectCounter::~GameObjectCounter() {
    id_ = -1;
    counter_ = nullptr;
    instance_flag_ = false;
}
