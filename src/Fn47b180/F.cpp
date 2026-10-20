// SQZ.EXE 0x47b180 (ret 0x30): NavMesh::Raycast - cast the segment origin + dir * length against the faces of the octree
// leaf holding its end point, nearest hit first; the first hit inside its face and of an allowed surface type gives the
// contact point, the face normal, a right/forward frame built from `up`, and the face index. 0 when nothing is hit.
#include <stdlib.h>

struct V3 {
    float x, y, z;
    V3() {}
    ~V3();                                                  // 0x401660
    V3 operator*(float s) const;                            // 0x401b60
    V3 operator+(const V3 &o) const;                        // 0x403070
    V3 Cross(const V3 &o) const;                            // 0x41a350
    float Normalize();                                      // 0x47d630
};
struct NavFace {                                            // 0x28
    int v[3];                                               // 0x00
    V3 normal;                                              // 0x0c
    char pad18[0x24 - 0x18];
    int type;                                               // 0x24
};
struct NavHit { int face; float t; };
struct NavBox { char pad[0x18]; int count; int *faces; };   // 0x18 / 0x1c
int Intersect(const V3 &from, const V3 &to, const V3 *vertex, const V3 *normal, float *t);   // 0x47d850 (cdecl)
int CompareHits(const void *a, const void *b);              // 0x47c630

struct NavMesh {
    int unknown00;
    V3 *vertices;                                           // 0x04
    int unknown08;
    NavFace *faces;                                         // 0x0c
    NavBox *root;                                           // 0x10
    NavHit hits[0x400];                                     // 0x14
    NavBox *FindLeaf(const V3 *point, NavBox *box);         // 0x47d560
    int Contains(NavFace *face, const V3 &point);           // 0x47c2a0
    int Raycast(const V3 &origin, int unused1, int unused2, const V3 &up, const V3 &dir, float length,
        V3 *point, V3 *right, V3 *normal, V3 *forward, int *face, const int *typeAllowed);
};

int NavMesh::Raycast(const V3 &origin, int unused1, int unused2, const V3 &up, const V3 &dir, float length,
    V3 *point, V3 *right, V3 *normal, V3 *forward, int *face, const int *typeAllowed)
{
    V3 end = origin + dir * length;
    NavBox *leaf = FindLeaf(&end, 0);
    if (!leaf)
        return 0;
    int count = leaf->count;
    int n = 0;
    int i;
    for (i = 0; i < count; i++) {
        NavFace *f = &faces[leaf->faces[i]];
        if (f->normal.z * dir.z + f->normal.y * dir.y + f->normal.x * dir.x < 0.0f) {
            float t;
            if (Intersect(origin, end, &vertices[f->v[0]], &f->normal, &t) && t >= -0.1f && t <= 1.5f) {
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
        V3 p = origin + dir * (hits[i].t * length);
        NavFace *f = &faces[hits[i].face];
        if (Contains(f, p) && typeAllowed[f->type]) {
            point->x = p.x; point->y = p.y; point->z = p.z;
            normal->x = f->normal.x; normal->y = f->normal.y; normal->z = f->normal.z;
            V3 r = normal->Cross(up);
            right->x = r.x; right->y = r.y; right->z = r.z;
            right->Normalize();
            V3 fw = right->Cross(*normal);
            forward->x = fw.x; forward->y = fw.y; forward->z = fw.z;
            *face = hits[i].face;
            return 1;
        }
    }
    return 0;
}
