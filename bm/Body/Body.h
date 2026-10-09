// Body: the physical state every actor in a simulation has. Only the five trivial getters are ported (0x40bb70-0x40bba0, 0x41fcb0).
// Layout from Repo/src/Body.h. Float3 is the plain 12-byte vector (no destructor).
#pragma once
#include "compat41.h"

struct Float3 { float x, y, z; };

struct Contact {
    int   face;          // 0x00
    int   second;        // 0x04
    float matrix[16];    // 0x08
};

struct Body {
    void **vtbl;         // 0x00
    float  matrix[16];   // 0x04
    Float3 facing;       // 0x44
    float  facingValue;  // 0x50
    Float3 angles;       // 0x54
    float  unused60[3];  // 0x60
    float  extraValue;   // 0x6c
    Contact contact;     // 0x70

    float  *GetMatrix();        // 0x41fcb0 slot 3
    Float3 *GetFacing();        // 0x40bb70 slot 4
    float   GetFacingValue();   // 0x40bb80 slot 5
    float   GetExtraValue();    // 0x40bb90 slot 6
    Contact *GetContact();      // 0x40bba0 slot 7
    int IsOnSurface();          // 0x40bbb0 slot 8 (base: always 0)
    void SetFacing(const Float3 *direction, float value); // 0x41fd30 slot 10
    void SetAngles(float x, float y, float z);            // 0x41fd60 slot 11
    void GetAngles(float *x, float *y, float *z);         // 0x41fd80 slot 12
    void SetMatrix(const float *m);                       // 0x41fcc0 slot 9
    void SetContact(const Contact *c);                    // 0x420060 slot 14
};
