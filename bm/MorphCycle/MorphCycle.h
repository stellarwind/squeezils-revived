// MorphCycle: a named animation clip over shared samples (SQZ.EXE 0x41b7b0-0x41c000), VC++ 4.1 dialect.
// Layout from Repo/src/Animation.h. This unit holds the five trivial getters (0x41ba00-0x41ba40); the
// ctors/dtor/TimeAt/SamplePose/BlendPose are larger and not ported yet.
#pragma once
#include "compat41.h"

struct MorphCycle {
    float duration;                 // 0x00
    float period;                   // 0x04
    float startTime;                // 0x08
    int   firstSample;              // 0x0c
    int   noLoop;                   // 0x10
    int   sampleCount;              // 0x14
    float speed;                    // 0x18
    int   sharesData;               // 0x1c
    void *times;                    // 0x20 PrimitiveArray<float,2>*
    void *parts;                    // 0x24 PartCycleTable*

    float Duration();               // 0x41ba00
    float Period();                 // 0x41ba10
    float Speed();                  // 0x41ba20
    int   SampleCount();            // 0x41ba30
    int   NoLoop();                 // 0x41ba40
    float TimeAt(int sample);       // 0x41ba50
};
