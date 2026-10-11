// 0x47d850: parameter t of the intersection of a ray with a plane. cdecl, 5 stack args.
#include "Free2/Geom.h"
static inline float Dot(const Vector3f &p, const Vector3f &q) { return p.x * q.x + p.y * q.y + p.z * q.z; }
int RayPlane(const Vector3f &a, const Vector3f &b, const Vector3f &c, const Vector3f &n, float *t)
{
    Vector3f d = b - a;
    float num = Dot(c - a, n);
    float den = Dot(n, d);
    if (den == 0.0f) return 0;
    *t = num / den;
    return 1;
}
