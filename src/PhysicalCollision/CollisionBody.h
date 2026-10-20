#pragma once
#include "Vector3f/Vector3f.h"

struct CollisionMatrix { Vector3f *Position(); }; // 0x41faf0
struct CollisionBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual CollisionMatrix *GetMatrix();           // +0x0c
    virtual Vector3f *Direction();                  // +0x10
    virtual float Speed();                         // +0x14
    virtual float Mass();                          // +0x18
    virtual void v7(); virtual void v8(); virtual void v9();
    virtual void SetMotion(Vector3f *direction, float speed); // +0x28
    char pad[0xbc - 4];
    float bounciness;                               // +0xbc
};
