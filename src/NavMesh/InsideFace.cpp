// 0x47c2a0 (ret 8): NavMesh::InsideFace - 1 when the point lies inside the face's three edge planes (on an edge
// counts as inside), 0 otherwise. The sign test reads the float bits, so -0 counts as 0.
#include "NavMesh/NavMesh.h"

static inline int BitSign(float f)
{
    int bits = *(int *)&f;
    if (bits > 0)
        return 1;
    return ((unsigned)bits <= 0x80000000u) - 1;
}

int NavMesh::InsideFace(MFace *face, const Vector3f &point)
{
    const MVert &v0 = points[face->a];
    Vector3f p0(v0.x, v0.y, v0.z);
    const MVert &v1 = points[face->b];
    Vector3f p1(v1.x, v1.y, v1.z);
    const MVert &v2 = points[face->c];
    Vector3f p2(v2.x, v2.y, v2.z);
    Vector3f e0 = p1 - p0;
    Vector3f e1 = p2 - p1;
    Vector3f e2 = p0 - p2;
    Vector3f normal(face->normal.x, face->normal.y, face->normal.z);
    Vector3f c0 = normal.Cross(e0);
    Vector3f c1 = normal.Cross(e1);
    Vector3f c2 = normal.Cross(e2);
    Vector3f d0 = point - p0;
    Vector3f d1 = point - p1;
    Vector3f d2 = point - p2;
    float dot[3];
    dot[0] = (d0.x * c0.x + d0.y * c0.y + d0.z * c0.z);
    dot[1] = (d1.x * c1.x + d1.y * c1.y + d1.z * c1.z);
    dot[2] = (d2.x * c2.x + d2.y * c2.y + d2.z * c2.z);
    int s0 = BitSign(dot[0]);
    int s1 = BitSign(dot[1]);
    int s2 = BitSign(dot[2]);
    int outside = s0 == -1 || s1 == -1 || s2 == -1;
    if (outside)
        return 0;
    return 1;
}
