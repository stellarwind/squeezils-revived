// Typed bridge from local 12-byte vector views to the shared source owner.
#pragma once
#include "NavMesh/NavMesh.h"

template<class V> inline int CastNavigationRay(NavMesh *mesh,
    const V &origin, const V &right, const V &up, const V &forward,
    const V &dir, float length, V *point, V *hitRight, V *hitUp,
    V *hitForward, int *face, const int *typeAllowed)
{
    // Original 0x47b180 ignores right/up; its fourth vector builds the frame.
    Vector3f start(origin.x, origin.y, origin.z);
    Vector3f frameForward(forward.x, forward.y, forward.z);
    Vector3f direction(dir.x, dir.y, dir.z);
    Vector3f p, r, n, f;
    int result = mesh->Raycast(start, 0, 0, frameForward, direction, length,
        &p, &r, &n, &f, face, typeAllowed);
    if (result) {
        point->x = p.x; point->y = p.y; point->z = p.z;
        hitRight->x = r.x; hitRight->y = r.y; hitRight->z = r.z;
        hitUp->x = n.x; hitUp->y = n.y; hitUp->z = n.z;
        hitForward->x = f.x; hitForward->y = f.y; hitForward->z = f.z;
    }
    return result;
}
