// SqueezilBody: a Body plus the animated character and clip state. Only the three trivial getters/setters are ported
// (0x457ac0 GetMode, 0x458fe0 IsOnGround, 0x458ff0 SetOnGround). Layout from Repo/legacy_gcc/SqueezilBody.h.
#pragma once
#include "Body/Body.h"

struct SqueezilBody : Body {
    float  collisionRadius;    // 0xb8
    float  collisionSpeedAmp;  // 0xbc
    float  lastMatrix[16];     // 0xc0
    float  shownMatrix[16];    // 0x100
    float  blendFrom[16];      // 0x140
    float  unused180[16];      // 0x180
    float  blendTo[16];        // 0x1c0
    float  blendTime;          // 0x200
    int    blending;           // 0x204
    int    noBlend;            // 0x208
    int    unknown20c;         // 0x20c
    Float3 orientation;        // 0x210
    float  clipSpeed;          // 0x21c
    void  *frame;              // 0x220
    int    onGround;           // 0x224
    int    unknown228;         // 0x228
    void  *character;          // 0x22c
    int    mode;               // 0x230
    int    previousMode;       // 0x234

    int GetMode();                              // 0x457ac0 slot 20
    int IsOnGround();                           // 0x458fe0 slot 15
    void SetOnGround(int on);                   // 0x458ff0 slot 16
    void SetOrientation(float pitch, float yaw, float roll);   // 0x457780 slot 17
    void GetOrientation(float *pitch, float *yaw, float *roll); // 0x4577b0 slot 18
    void SetClipSpeed(float s);                 // 0x459010 slot 24
    float GetClipSpeed();                       // 0x459020 slot 25
    float GetPitch();                           // 0x459030 slot 26
    float GetRoll();                            // 0x459040 slot 27
};

const char *BodyColorName(int color);           // 0x458fd0
