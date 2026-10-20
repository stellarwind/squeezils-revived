// 0x4026e0: wander toward the first corner of a random face on the current surface.
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"
#include "World/World.h"
#include <stdlib.h>
struct PickMatrix {
    float m[16];
    PickMatrix(const PickMatrix &o) {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
    }
    ~PickMatrix(); // 0x402310
};
struct PickContact { int face, second; PickMatrix matrix; ~PickContact(); }; // 0x4031e0
struct PickFace {
    int vertex[3];
    Vector3f normal;
    int neighbour[3], type;
    ~PickFace(); // 0x4053e0
};
struct NavMesh { void *vtable; Vector3f *points; unsigned faceCount; PickFace *faces; };
struct PickBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual const PickContact *GetContact();
};
struct PickTurn { char pad[0x2bc]; Vector3f target; };
World *GetPickWorld(); // 0x4206a0
struct WanderPick {
    void *vtable;
    PickBody *body;
    char pad08[0x1c - 8];
    PickTurn *turn;
    void PickTarget();
};
void WanderPick::PickTarget()
{
    PickContact contact = *body->GetContact();
    NavMesh *mesh = GetPickWorld()->SurfacesOf(contact.face);
    PickFace face = mesh->faces[(unsigned)rand() % mesh->faceCount];
    Vector3f point = mesh->points[face.vertex[0]];
    PickTurn *destination = turn;
    destination->target.x = point.x;
    destination->target.y = point.y;
    destination->target.z = point.z;
}
