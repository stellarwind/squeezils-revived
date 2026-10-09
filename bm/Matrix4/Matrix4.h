// Matrix4: the 4x4 float matrix used by the transform code (SQZ.EXE 0x47dcd0-0x47e330), VC++ 4.1 dialect.
// All members are thiscall, return *this, and set the matrix element-wise (the original is not a loop or memset).
//
// Status (score = instruction diff, 0 = match):
//   SetTranslation 0x47e330  0
//   TurnX          0x47dcd0  14 (x87 scheduling: cos/sin store order)
//   TurnY          0x47dd20  9  (same)
//   TurnZ          0x47dd70  12 (same)
//   TurnAxis       0x47ddc0  24 in context, 8 in isolation (Rodrigues rotation; scheduling)
#pragma once
#include "compat41.h"

struct Vec3 {
    float x, y, z;
};

struct Matrix4 {
    float m[16];

    Matrix4 &SetTranslation(const float *t);   // 0x47e330
    Matrix4 &TurnX(float angle);               // 0x47dcd0
    Matrix4 &TurnY(float angle);               // 0x47dd20
    Matrix4 &TurnZ(float angle);               // 0x47dd70
    Matrix4 &TurnAxis(const Vec3 &a, float angle);   // 0x47ddc0
};
