// 15 spare declarations (symbol count, see padscan.sh): steer the x87 order of the first dot product.
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
// 0x47d850: parameter t of the intersection of a ray with a plane. cdecl, 5 stack args.
#include "Free2/Geom.h"
static inline float Dot(const Vector3f &p, const Vector3f &q) { return p.x * q.x + p.y * q.y + p.z * q.z; }
int RayPlane(const Vector3f &a, const Vector3f &b, const Vector3f &c, const Vector3f &n, float *t)
{
    Vector3f d = b - a;
    float num = Dot(n, c - a);
    float den = Dot(n, d);
    if (den == 0.0f) return 0;
    *t = num / den;
    return 1;
}
