// Matrix4: the 4x4 float matrix used by the transform code (SQZ.EXE 0x47dcd0-0x47e330), VC++ 4.1 dialect.
// All members are thiscall, return *this, and set the matrix element-wise (the original is not a loop or memset).
//
// Status (score = instruction diff, 0 = match):
//   SetTranslation 0x47e330  0
//   TurnX          0x47dcd0  0 (fixed: inline cos/sin instead of c/s locals; note TurnY uses the opposite sign
//                              convention m[2]=-sin/m[8]=+sin)
//   TurnY          0x47dd20  0
//   TurnZ          0x47dd70  0
//   TurnAxis       0x47ddc0  24 in context, 8 in isolation (Rodrigues rotation; scheduling)
#pragma once
#include "compat41.h"

struct Vec3 {
    float x, y, z;
};

struct Matrix4 {
    float m[16];

    Matrix4 &SetTranslation(const float *t);   // 0x47e330
    Matrix4 &SetRows(const Vec3 &right, const Vec3 &up,
                     const Vec3 &forward, const Vec3 &position); // 0x409f00
    Matrix4 &TurnX(float angle);               // 0x47dcd0
    Matrix4 &TurnY(float angle);               // 0x47dd20
    Matrix4 &TurnZ(float angle);               // 0x47dd70
    Matrix4 &TurnAxis(const Vec3 &a, float angle);   // 0x47ddc0
};
