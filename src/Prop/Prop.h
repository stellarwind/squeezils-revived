// Prop: a placed thing with a D3DRM frame. Only IsShown (0x426160) is ported. Layout from Repo/legacy_gcc/Prop.h.
#pragma once
#include "compat41.h"

#ifdef PROP_CONSTRUCTOR
#include "Vector3f/Vector3f.h"
typedef Vector3f PropVector3;
#else
struct Float3 { float x, y, z; };
typedef Float3 PropVector3;
#endif

struct Prop {
    void  **vtbl;       // 0x00
    PropVector3 position;    // 0x04
    PropVector3 scale;       // 0x10
    PropVector3 direction;   // 0x1c
    PropVector3 up;          // 0x28
    void  *frame;       // 0x34 (NULL while hidden)
    void  *owner;       // 0x38
    int    isInstance;  // 0x3c

#ifdef PROP_CONSTRUCTOR
    Prop(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up); // 0x425e50
#endif

    int IsShown();         // 0x426160 slot 14
    void *GetOwner();      // 0x426220 slot 9
    void UpdateNoop(float seconds);   // 0x426200 slot 7 (no-op)
    void Slot8Noop();      // 0x426210 slot 8 (no-op)
    int  Slot11Return0(int a);        // 0x4262d0 slot 11 (returns 0)
    int  Slot12Return0();  // 0x426270 slot 12 (returns 0)
};
