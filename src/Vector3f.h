// Vector3f: three floats and the few operations the animation code uses. The game has its own class of this name (RTTI shows
// Array<Vector3f> and PrimitiveArray<Vector3f>); its operators are separate small functions in the original (0x401ae0 minus,
// 0x401b60 times a number, 0x403070 plus, 0x41a350 cross product, 0x47d5f0 length, 0x47d630 normalize), reconstructed here.
//
// Why it has an (empty) destructor: the original class had one, and a heap array of such elements starts with a count word
// (NewArray in Container.h does that for types with a destructor). Arrays made here are freed by original code and the other
// way round, so both sides must agree on the layout. For the same reason a Vector3f is never passed by value to original code
// (a compiler passes a type with a destructor differently); Float3 (Float3.h) is the plain version for that.
#pragma once
#include <math.h>

struct Vector3f {
    float x, y, z;

    Vector3f() {}                      // the original constructor does nothing (@00457040)
    Vector3f(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    Vector3f(const Vector3f &o) : x(o.x), y(o.y), z(o.z) {}
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    ~Vector3f() {}                     // @00401660
};

inline Vector3f operator+(const Vector3f &a, const Vector3f &b) { return Vector3f(a.x + b.x, a.y + b.y, a.z + b.z); }   // @00403070
inline Vector3f operator-(const Vector3f &a, const Vector3f &b) { return Vector3f(a.x - b.x, a.y - b.y, a.z - b.z); }   // @00401ae0
inline Vector3f operator*(const Vector3f &a, float s)           { return Vector3f(a.x * s, a.y * s, a.z * s); }          // @00401b60

// a x b                                                                                                                   @0041a350
inline Vector3f Cross(const Vector3f &a, const Vector3f &b)
{
    return Vector3f(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

inline float Length(const Vector3f &v) { return sqrtf(v.z * v.z + v.y * v.y + v.x * v.x); }                              // @0047d5f0

// Scales v to length 1 (unless its length is 0) and returns the length it had.                                          @0047d630
inline float Normalize(Vector3f &v)
{
    float length = Length(v);
    if (length != 0.0f) {
        float scale = 1.0f / length;
        v.x *= scale;
        v.y *= scale;
        v.z *= scale;
    }
    return length;
}
