// Shared 0x40c700 nearness query and the prefix fields it reads.
#pragma once
#include <stddef.h>
#include "Vector3f/Vector3f.h"

// SIZE 0x08: surface-data prefix used by the query.
struct NearSurfaceData {
    void *mesh; // 0x00
    float sizeSquared; // 0x04
};

// Prefix view through +0xe7; the complete instance has further fields.
struct NearSurfaceInstance {
    char pad00[0xdc]; // 0x00
    Vector3f position; // 0xdc
    NearSurfaceData *GetSurface(); // 0x40c6b0
    int IsNear(const Vector3f &point); // 0x40c700, NavSurface/IsNear.cpp
};
typedef char NearPositionOffsetCheck[(offsetof(NearSurfaceInstance, position) == 0xdc) ? 1 : -1];
typedef char NearRadiusOffsetCheck[(offsetof(NearSurfaceData, sizeSquared) == 4) ? 1 : -1];

template<class V> inline int IsNavigationSurfaceNear(void *surface, const V &point)
{
    Vector3f p(point.x, point.y, point.z);
    return ((NearSurfaceInstance *)surface)->IsNear(p);
}
