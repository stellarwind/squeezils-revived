// SqueezilSoul::GetGroundMotion (0x456b90): copy the state's way-of-moving settings into the control record.
#pragma once
#include "compat41.h"

struct GroundMotion { float v[6]; };    // 0x18
struct LeverMotion  { float v[11]; };   // 0x2c

struct GGMPhysics {
    char pad0[0x24];
    GroundMotion walk;    // 0x24
    GroundMotion run;     // 0x3c
    GroundMotion scamper; // 0x54
    GroundMotion skid;    // 0x6c
    char pad1[0x210 - 0x84];
    LeverMotion attack;   // 0x210
    LeverMotion slip;     // 0x23c
};

struct GGMControl {
    float forward;        // 0x00
    float turn;           // 0x04
    union {
        GroundMotion ground;
        LeverMotion  lever;
    };                    // 0x08
};

struct GGMSoul {
    char pad[0x48];
    GGMPhysics *physics;  // 0x48

    void GetGroundMotion(int forState, GGMControl *c);  // 0x456b90
};
