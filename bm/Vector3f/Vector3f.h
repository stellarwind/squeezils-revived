// Vector3f: three floats and the basic operations the game uses. Reconstructed from SQZ.EXE as thiscall members
// (the originals are member functions, this in ecx, not the free inline forms in Repo/src). Byte-match track, VC++ 4.1.
//
// Functions and status (score = instruction diff, 0 = match):
//   ctor(3f)   0x418570  0 (real function; see Vector3fCtor.cpp + V3_CTOR_OUTOFLINE)
//   operator=  0x4183c0  0
//   operator-  0x401ae0  0
//   operator*  0x401b60  0
//   operator+  0x403070  open (x87 scheduling order)
//   Cross      0x41a350  open (x87 scheduling order)
//   Length     0x47d5f0  open (x87 scheduling order: original z,y,x, ours x,z,y -- see BYTEMATCH_STATUS.md)
//   LengthSq   0x47d610  0 or 4 (scheduling leak, see BYTEMATCH_STATUS.md)
//   Normalize  0x47d630  0
//
// The empty destructor is required: a type with a destructor is returned/passed differently, and the operators
// return Vector3f by value (hidden result pointer). Keep it, plus the 3-arg constructor.
#pragma once
#include "compat41.h"

struct Vector3f {
    float x, y, z;

    Vector3f() {}
#ifdef V3_CTOR_OUTOFLINE
    Vector3f(float ax, float ay, float az);          // out-of-line, 0x418570
#else
    Vector3f(float ax, float ay, float az) : x(ax), y(ay), z(az) {}   // inlined by the operators
#endif
    ~Vector3f() {}

#ifdef V3_ASSIGN_OUTOFLINE
    Vector3f &operator=(const Vector3f &o);       // out-of-line, 0x4183c0 (vector copy)
#else
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }   // inlined where a vector is copied
#endif
    Vector3f operator-(const Vector3f &o) const;   // 0x401ae0
    Vector3f operator*(float s) const;             // 0x401b60
    Vector3f operator+(const Vector3f &o) const;   // 0x403070
    Vector3f Cross(const Vector3f &o) const;       // 0x41a350
    float Length() const;                          // 0x47d5f0
    float LengthSq() const;                        // 0x47d610
    float Normalize();                             // 0x47d630
};
