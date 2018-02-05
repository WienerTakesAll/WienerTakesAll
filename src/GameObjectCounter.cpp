#include "GameObjectCounter.h"

int GameObjectCounter::id_ = -1;
bool GameObjectCounter::instance_flag_ = false;
GameObjectCounter* GameObjectCounter::counter_ = nullptr;

GameObjectCounter* GameObjectCounter::get_instance() {
    if (!instance_flag_) {
        counter_ = new GameObjectCounter();
        instance_flag_ = true;
        id_ = 0;
    }

    return counter_;
}

int GameObjectCounter::assign_id() const {
    return id_++;
}

int GameObjectCounter::id() const {
    return id_;
}

GameObjectCounter::~GameObjectCounter() {
    counter_ = nullptr;
    instance_flag_ = false;
}
