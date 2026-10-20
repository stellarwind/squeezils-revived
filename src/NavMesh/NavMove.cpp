// SQZ.EXE 0x47b490 (ret 0x38, recursive): NavMesh::Move - walk `distance` along the surface from `pos` on face `face`.
// The direction is flattened onto the face; when the end point is still inside the face the frame is kept and 1 returned.
// Otherwise the path is cut at the nearest crossed edge: a missing neighbour or a surface type not allowed by `typeAllowed`
// is a wall (stop on the edge, *newFace = -1, return 0); else the frame is turned about the edge by the angle between the
// two faces and the walk goes on into the neighbour with what is left (at most 16 faces deep, g_navMoveDepth).
#include <math.h>

struct V3 {
    float x, y, z;
    V3() {}
    V3(float ax, float ay, float az) { x = ax; y = ay; z = az; }
    ~V3();                                                  // 0x401660
    V3 operator-(const V3 &o) const;                        // 0x401ae0
    V3 operator*(float s) const;                            // 0x401b60
    V3 operator+(const V3 &o) const;                        // 0x403070
    V3 Cross(const V3 &o) const;                            // 0x41a350
    float Normalize();                                      // 0x47d630
};
// SIZE 0x40
struct M4 {
    float m[16];
    M4() {}
    ~M4();                                                  // 0x402310
    M4 *Make(const V3 &right, const V3 &up, const V3 &forward, const V3 &pos);   // 0x412830
    M4 &TurnAxis(const V3 &axis, float angle);              // 0x47ddc0
    M4 operator*(const M4 &o) const;                        // 0x47dea0
};
V3 Transform(const V3 &p, const M4 &m);                     // 0x47e1e0 (cdecl)
int ClosestLineParams(const V3 *p1, const V3 *p2, const V3 *p3, const V3 *p4, float *s, float *t);   // 0x47d680 (cdecl)

// SIZE 0x28
struct NavFace {
    int v[3];                                               // 0x00
    V3 normal;                                              // 0x0c
    int neighbour[3];                                       // 0x18 (-1: open edge)
    int type;                                               // 0x24
};

struct NavMesh {
    int unknown00;
    V3 *vertices;                                           // 0x04
    int unknown08;
    NavFace *faces;                                         // 0x0c
    int Move(const V3 &pos, const V3 &right, const V3 &up, const V3 &forward, const V3 &dir, float distance, int face,
        V3 *newPos, V3 *newRight, V3 *newUp, V3 *newForward, V3 *newDir, int *newFace, const int *typeAllowed);
};

int g_navMoveDepth;                                         // 0x4c3df4

static inline int Side(float v)
{
    if (v > 1e-5f)
        return 1;
    return v < -1e-5f ? -1 : 0;
}

int NavMesh::Move(const V3 &pos, const V3 &right, const V3 &up, const V3 &forward, const V3 &dir, float distance, int face,
    V3 *newPos, V3 *newRight, V3 *newUp, V3 *newForward, V3 *newDir, int *newFace, const int *typeAllowed)
{
    if (g_navMoveDepth > 16)
        return 0;
    if (distance > -0.005f && distance < 0.005f) {
        *newPos = pos;
        *newRight = right;
        *newUp = up;
        *newForward = forward;
        *newDir = dir;
        *newFace = face;
        return 1;
    }

    NavFace *here = &faces[face];
    const V3 *normal = &here->normal;
    V3 across = normal->Cross(dir);
    across.Normalize();
    V3 slide = across.Cross(*normal);
    V3 target = pos + slide * distance;

    V3 c0 = vertices[here->v[0]];
    V3 c1 = vertices[here->v[1]];
    V3 c2 = vertices[here->v[2]];
    V3 e0 = c1 - c0;
    V3 e1 = c2 - c1;
    V3 e2 = c0 - c2;
    V3 n0 = normal->Cross(e0);
    V3 n1 = normal->Cross(e1);
    V3 n2 = normal->Cross(e2);
    V3 d0 = target - c0;
    V3 d1 = target - c1;
    V3 d2 = target - c2;
    float k0 = d0.z * n0.z + d0.y * n0.y + d0.x * n0.x;
    float k1 = d1.z * n1.z + d1.y * n1.y + d1.x * n1.x;
    float k2 = d2.z * n2.z + d2.y * n2.y + d2.x * n2.x;
    int s0 = Side(k0);
    int s1 = Side(k1);
    int s2 = Side(k2);

    if (s0 != -1 && s1 != -1 && s2 != -1) {                 // still on this face
        *newPos = target;
        *newRight = right;
        *newUp = up;
        *newForward = forward;
        *newDir = slide;
        *newFace = face;
        return 1;
    }

    // Where the path crosses each edge it is outside of: as a part of the path and as a part of the edge
    float along[3];
    float onEdge[3];
    along[2] = along[1] = along[0] = onEdge[2] = onEdge[1] = onEdge[0] = 1000000.0f;
    if (s0 == -1)
        ClosestLineParams(&pos, &target, &c0, &c1, &along[0], &onEdge[0]);
    if (s1 == -1)
        ClosestLineParams(&pos, &target, &c1, &c2, &along[1], &onEdge[1]);
    if (s2 == -1)
        ClosestLineParams(&pos, &target, &c2, &c0, &along[2], &onEdge[2]);
    int crossed = 0;
    int i;
    for (i = 1; i < 3; i++)
        if (along[i] < along[crossed])
            crossed = i;
    V3 edgePoint;
    switch (crossed) {
    case 0:
        edgePoint = c0 + e0 * onEdge[0];
        break;
    case 1:
        edgePoint = c1 + e1 * onEdge[1];
        break;
    case 2:
        edgePoint = c2 + e2 * onEdge[2];
        break;
    }

    int next = here->neighbour[crossed];
    NavFace *there = &faces[next];
    int allowed = typeAllowed[there->type];
    if (next == -1 || allowed == 0) {      // a wall: stop on the edge
        *newRight = right;
        *newUp = up;
        *newForward = forward;
        *newPos = edgePoint;
        *newDir = slide;
        *newFace = -1;
        return 0;
    }

    // Over the edge: turn the frame about the edge by the angle between the two faces
    V3 axis = crossed == 0 ? e0 : crossed == 1 ? e1 : e2;
    axis.Normalize();
    float angle = here->normal.z * there->normal.z + here->normal.y * there->normal.y + there->normal.x * normal->x;
    if (angle > 1.0f)
        angle = 1.0f;
    angle = (float)acos(angle);
    float slope = there->normal.z * slide.z + there->normal.x * slide.x + there->normal.y * slide.y;
    if (distance < 0.0f)
        slope = -slope;
    if (slope < 0.0f)
        angle = -angle;
    M4 turn;
    turn.TurnAxis(axis, angle);
    M4 frame;
    V3 zero(0.0f, 0.0f, 0.0f);
    frame.Make(right, up, forward, zero);
    M4 turned = frame * turn;
    V3 nextRight(turned.m[0], turned.m[1], turned.m[2]);
    V3 nextUp(turned.m[4], turned.m[5], turned.m[6]);
    V3 nextForward(turned.m[8], turned.m[9], turned.m[10]);
    V3 nextDir = Transform(slide, turn);

    float walked = along[crossed] * distance;
    float remaining = distance - walked;
    if (remaining > -0.005f && remaining < 0.005f) {        // the edge was the end of the walk
        *newRight = nextRight;
        *newUp = nextUp;
        *newForward = nextForward;
        *newPos = edgePoint;
        *newDir = nextDir;
        *newFace = next;
        return 1;
    }
    g_navMoveDepth++;
    int result = Move(edgePoint, nextRight, nextUp, nextForward, nextDir, remaining, next,
        newPos, newRight, newUp, newForward, newDir, newFace, typeAllowed);
    g_navMoveDepth--;
    return result;
}
