// AnimatedBody: the player object that moves the body parts (SQZ.EXE 0x41d9e0-0x41e30d), VC++ 4.1 dialect.
// Layout from Repo/src/BodyPart.h. Only the trivial getters/setter are ported (the class is polymorphic; these bodies are leaf).
#pragma once
#include "MorphCycle/MorphCycle.h"

struct AnimatedBody {
    void        *vtbl;           // 0x00
    char         partTable[0x28]; // 0x04 BodyPartTable
    MorphCycle  *cycle;          // 0x2c
    float        time;           // 0x30
    int          hint;           // 0x34
    int          direction;      // 0x38
    int          loopMode;       // 0x3c
    void        *pose;           // 0x40
    void        *previousPose;   // 0x44
    float        blendLeft;      // 0x48
    void        *rootFrame;      // 0x4c

    float GetTime();             // 0x41da70
    float GetDuration();         // 0x41da80
    int   GetLoopMode();         // 0x41da90
    void *GetRootFrame();        // 0x41daa0
    void  SetTime(float t);      // 0x41e290
    void  SetDirection(int d);   // 0x41e2d0 (only the sign counts)
    void  SetLoopMode(int m);    // 0x41e2f0 (only the sign counts)
};
