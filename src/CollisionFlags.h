#pragma once

namespace CollisionFlags {
    const unsigned int GROUND = 1 << 0;
    const unsigned int WHEEL = 1 << 1;
    const unsigned int CHASSIS = 1 << 2;
    const unsigned int OBSTACLE = 1 << 3;
    const unsigned int DRIVABLE_OBSTACLE = 1 << 4;

    const unsigned int GROUND_AGAINST =
        CHASSIS |
        OBSTACLE |
        DRIVABLE_OBSTACLE;

    const unsigned int WHEEL_AGAINST =
        WHEEL |
        CHASSIS |
        OBSTACLE;

    const unsigned int CHASSIS_AGAINST =
        GROUND |
        WHEEL |
        CHASSIS |
        OBSTACLE |
        DRIVABLE_OBSTACLE;


    const unsigned int DRIVABLE_SURFACE = 0xffff0000;
    const unsigned int UNDRIVABLE_SURFACE = 0x0000ffff;
}
