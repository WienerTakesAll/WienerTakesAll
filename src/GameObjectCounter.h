#pragma once

class GameObjectCounter {
public:
    static GameObjectCounter* get_instance();
    int assign_id() const;
    ~GameObjectCounter();

private:
    static int id_;
    static bool instance_flag_;
    static GameObjectCounter* counter_;
    GameObjectCounter() {
    }

};