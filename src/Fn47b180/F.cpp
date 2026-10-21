// SQZ.EXE 0x47b180 (ret 0x30): NavMesh::Raycast - cast the segment origin + dir * length against the faces of the octree
// leaf holding its end point, nearest hit first; the first hit inside its face and of an allowed surface type gives the
// contact point, the face normal, a right/forward frame built from `up`, and the face index. 0 when nothing is hit.
#include <stdlib.h>

#include "NavMesh/NavMesh.h"
struct NavHit { int face; float t; }; // SIZE 0x08, hitStorage at mesh+0x14
int RayPlane(const Vector3f &from, const Vector3f &to, const Vector3f &vertex,
    const Vector3f &normal, float *t); // 0x47d850, Free2/RayPlane.cpp
int CompareHits(const void *a, const void *b); // 0x47c630, NavMesh/CompareHits.cpp

int NavMesh::Raycast(const Vector3f &origin, int unused1, int unused2, const Vector3f &up, const Vector3f &dir, float length,
    Vector3f *point, Vector3f *right, Vector3f *normal, Vector3f *forward, int *face, const int *typeAllowed)
{
    NavHit *hits = (NavHit *)hitStorage;
    Vector3f end = origin + dir * length;
    BoxNode *leaf = FindLeaf(&end, 0);
    if (!leaf)
        return 0;
    int count = leaf->count;
    int n = 0;
    int i;
    for (i = 0; i < count; i++) {
        MFace *f = &faces[leaf->faces[i]];
        if (f->normal.z * dir.z + f->normal.y * dir.y + f->normal.x * dir.x < 0.001f) {
            float t;
            const MVert &v = points[f->a];
            Vector3f vertex(v.x, v.y, v.z);
            Vector3f planeNormal(f->normal.x, f->normal.y, f->normal.z);
            if (RayPlane(origin, end, vertex, planeNormal, &t) && t >= -0.1f && t <= 1.5f) {
                if (n >= 0x400)
                    break;
                hits[n].face = leaf->faces[i];
                hits[n].t = t;
                n++;
            }
        }
    }
    qsort(hits, n, sizeof(NavHit), CompareHits);
    for (i = 0; i < n; i++) {
        Vector3f p = origin + dir * (hits[i].t * length);
        MFace *f = &faces[hits[i].face];
        if (InsideFace(f, p) && typeAllowed[f->pad]) {
            point->x = p.x; point->y = p.y; point->z = p.z;
            normal->x = f->normal.x; normal->y = f->normal.y; normal->z = f->normal.z;
            Vector3f r = normal->Cross(up);
            right->x = r.x; right->y = r.y; right->z = r.z;
            right->Normalize();
            Vector3f fw = right->Cross(*normal);
            forward->x = fw.x; forward->y = fw.y; forward->z = fw.z;
            *face = hits[i].face;
            return 1;
        }
    }
    return 0;
}
