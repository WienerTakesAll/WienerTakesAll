#pragma once

enum CarControlType {
    FORWARD_DRIVE,
    BRAKE,
    STEER,
    HAND_BRAKE
};

struct CarControls {
    float forward_drive;
    float horizontal_drive;
    float braking_force;
    bool hand_break;
};
