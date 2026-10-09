// SqueezilSoul::GetAirMotion (0x456ca0): copy the state's air-motion settings into the control record.
#pragma once
#include "compat41.h"

struct FlightModel { float v[23]; };    // 0x5c
struct JumpModel   { float v[10]; };    // 0x28

struct GGAPhysics {
    char pad0[0x84];
    FlightModel fly;     // 0x84
    FlightModel dive;    // 0xe0
    FlightModel tumble;  // 0x13c
    JumpModel   jump;    // 0x198
    JumpModel   hop;     // 0x1c0
    JumpModel   skihop;  // 0x1e8
};

struct GGAControl {
    float pitch;         // 0x00
    float turn;          // 0x04
    union {
        FlightModel flight;
        JumpModel   jump;
    };                   // 0x08
};

struct GGASoul {
    char pad[0x48];
    GGAPhysics *physics;  // 0x48

    void GetAirMotion(int forState, GGAControl *c);  // 0x456ca0
};
