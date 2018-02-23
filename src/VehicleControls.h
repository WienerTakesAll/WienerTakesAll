#pragma once

enum VehicleControlType {
    FORWARD_DRIVE,
    BRAKE,
    STEER,
    HAND_BRAKE
};

struct VehicleControls {
    float forward_drive;
    float horizontal_drive;
    float braking_force;
    bool hand_break;
};
