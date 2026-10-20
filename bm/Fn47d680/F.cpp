// 0x47d680: closest approach of two lines; *s, *t are the parameters along each (0 if parallel).
#include "Vector3f/Vector3f.h"
#include <math.h>
int ClosestLineParams(const Vector3f *p1, const Vector3f *p2, const Vector3f *p3, const Vector3f *p4, float *s, float *t)
{
    Vector3f a = *p2 - *p1;
    float lengthA = a.Normalize();
    Vector3f b = *p4 - *p3;
    float lengthB = b.Normalize();
    Vector3f c = a.Cross(b);
    float cLength = c.Length();
    float cSquared = cLength * cLength;
    if (fabs(cSquared) < 1e-8)
        return 0;
    Vector3f w = *p3 - *p1;
    
    
    *s = (w.x * (b.y * c.z - b.z * c.y) + w.y * (b.z * c.x - b.x * c.z) + w.z * (b.x * c.y - b.y * c.x)) / (lengthA * cSquared);
    *t = (w.x * (a.y * c.z - a.z * c.y) + w.y * (a.z * c.x - a.x * c.z) + w.z * (a.x * c.y - a.y * c.x)) / (lengthB * cSquared);
    return 1;
}
