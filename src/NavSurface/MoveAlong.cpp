// SQZ.EXE 0x411120: NavSurfaces::MoveAlong - carry a body `distance` along the surface it stands on (the contact's
// surface), in that surface's local frame, then see whether the path runs into any other live surface near it.
// The result is in world space. Returns 1 (hit: the body stops on another surface, contact = that surface) or the first
// move's result (moved on the contact's own surface). Hits on a fly-through surface run the surface's script and are ignored.
#include <string.h>
#pragma intrinsic(strlen)
#include "compat41.h"
#include "NavMesh/RaycastBridge.h"
#include "NavSurface/Nearness.h"

struct SqueezilSoul;

struct MoveVector {
    float x, y, z;
    MoveVector() {}
    ~MoveVector();                                              // 0x401660
    MoveVector operator*(float scale) const;                    // 0x401b60
    MoveVector operator+(const MoveVector &v) const;            // 0x403070
    MoveVector operator-(const MoveVector &v) const;            // 0x401ae0
    float Length() const;                                       // 0x47d5f0
    float Normalize();                                          // 0x47d630
};

inline float Dot(const MoveVector &a, const MoveVector &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

// SIZE 0x40
struct MoveMatrix {
    float m[16];
    MoveMatrix() {}
    MoveMatrix(const MoveVector &right, const MoveVector &up, const MoveVector &forward, const MoveVector &position) {
        m[0] = right.x; m[1] = right.y; m[2] = right.z; m[3] = 0;
        m[4] = up.x; m[5] = up.y; m[6] = up.z; m[7] = 0;
        m[8] = forward.x; m[9] = forward.y; m[10] = forward.z; m[11] = 0;
        m[12] = position.x; m[13] = position.y; m[14] = position.z; m[15] = 1.0f;
    }
    MoveMatrix(const MoveMatrix &o) { Copy(o); }
    MoveMatrix &operator=(const MoveMatrix &o) { Copy(o); return *this; }
    void Copy(const MoveMatrix &o) {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
    }
    ~MoveMatrix();                                              // 0x402310
    MoveMatrix &SetTranslation(const MoveVector &v);            // 0x47e330
    MoveMatrix &SetRows(const MoveVector &right, const MoveVector &up, const MoveVector &forward,
                        const MoveVector &position);            // 0x409f00
    const MoveVector &Right() const { return *(const MoveVector *)(m + 0); }
    const MoveVector &Up() const { return *(const MoveVector *)(m + 4); }
    const MoveVector &Forward() const { return *(const MoveVector *)(m + 8); }
    const MoveVector &Position() const { return *(const MoveVector *)(m + 12); }
};

// The same matrix built by the out-of-line maker.
struct MoveFrame : MoveMatrix {
    MoveFrame(const MoveVector &right, const MoveVector &up, const MoveVector &forward,
              const MoveVector &position);                      // 0x412830
};

// SIZE 0x48
struct MoveContact {
    int face;                                                   // 0x00 surface number
    int second;                                                 // 0x04 face of that surface's mesh
    MoveMatrix matrix;                                          // 0x08
    MoveContact(int f, int s) : face(f), second(s) {}
    ~MoveContact();                                             // 0x4031e0
};

struct MoveMesh {
    // Walk `distance` along the surface (NavMesh::Move).
    int Move(const MoveVector &pos, const MoveVector &right, const MoveVector &up, const MoveVector &forward,
        const MoveVector &dir, float distance, int face, MoveVector *newPos, MoveVector *newRight,
        MoveVector *newUp, MoveVector *newForward, MoveVector *newDir, int *newFace,
        const int *typeAllowed);                                // 0x47b490
    // Cast the segment origin + dir * length (NavMesh::Raycast).

};
struct MoveSurfaceData {
    MoveMesh *mesh;                                             // 0x00
};

struct MoveSurface {
    MoveSurfaceData *GetSurface();                              // 0x40c6b0
    const int *TypeFilter();                                    // 0x40c6d0
    int WasNear(const MoveVector &point);                       // 0x40c7b0
    int Changed();                                              // 0x40c860
    MoveVector ToLocal(const MoveVector &point);                // 0x40f6f0
    MoveVector ToWorld(const MoveVector &point);                // 0x40f850
    MoveMatrix ToWorld(const MoveMatrix &matrix);               // 0x4105f0
    MoveVector ToLocalBefore(const MoveVector &point);          // 0x410660
    MoveVector ToWorldBefore(const MoveVector &point);          // 0x4128a0
    MoveMatrix Relative(const MoveMatrix &matrix);              // 0x4110b0
};

// SIZE 0x10
struct MoveSurfaces {
    virtual MoveSurface *Get(int index);                        // vtable+0x00
    int count;                                                  // 0x04
    int capacity;                                               // 0x08
    MoveSurface **data;                                         // 0x0c
};

class String {
public:
    ~String();                                                  // 0x4383d0
    const char *Chars();                                        // 0x438530
    char *rep;
};

struct ScriptCaller {
    int data[3];
    void InitSoul(SqueezilSoul *soul, int actorIndex, void *prop);     // 0x420900
};
#include "Script/CallFunction.h"

struct Simulation {
    int FindIdBySoul(void *soul);                               // 0x4140b0
};
Simulation *GetSimulation();                                    // 0x420690

struct NavSurfaces {
    char pad[0x134];
    MoveSurfaces instances;                                     // 0x134
    int MoveAlong(SqueezilSoul *soul, MoveVector *position, MoveVector *right, MoveVector *up, MoveVector *forward,
        MoveVector *direction, float distance, MoveContact *contact, MoveVector *newPosition, MoveVector *newRight,
        MoveVector *newUp, MoveVector *newForward, MoveVector *newDirection, MoveContact *newContact);   // 0x411120
    int FlyThrough(MoveContact *c);                             // 0x40f8d0
    String ActivateScript(MoveContact *c);                      // 0x40f9b0
};

// Back to the first move's result (in the contact surface's local frame).
#define RESTORE_FIRST_MOVE() \
    newPos = hitLocal.Position(); \
    newRight2 = hitLocal.Right(); \
    newUp2 = hitLocal.Up(); \
    newForward2 = hitLocal.Forward(); \
    newDir = newDirSaved; \
    newFace = newFaceSaved; \
    contactFace = contact->face; \
    cur = instances.Get(contact->face)

#define RUN_SURFACE_SCRIPT() \
    String script = ActivateScript(&c); \
    if (strlen(script.Chars()) != 0) { \
        ScriptCaller caller; \
        caller.InitSoul(soul, GetSimulation()->FindIdBySoul(soul), 0); \
        CallFunction(script.Chars(), &caller, 0, 0); \
    }

int NavSurfaces::MoveAlong(SqueezilSoul *soul, MoveVector *position, MoveVector *right, MoveVector *up,
    MoveVector *forward, MoveVector *direction, float distance, MoveContact *contact, MoveVector *newPosition,
    MoveVector *newRight, MoveVector *newUp, MoveVector *newForward, MoveVector *newDirection,
    MoveContact *newContact)
{
    MoveSurface *cur = instances.Get(contact->face);            // the surface the body stands on
    int contactFace = contact->face;
    MoveMatrix frame(*right, *up, *forward, *position);
    MoveVector end = *position + *direction * distance;
    MoveVector localEnd = cur->ToLocal(end);
    MoveMatrix cm = contact->matrix;
    MoveVector dir = localEnd - cm.Position();
    dir.Normalize();

    // First walk along the contact's own surface.
    MoveVector newPos, newRight2, newUp2, newForward2, newDir;
    int newFace;
    int movedOk = cur->GetSurface()->mesh->Move(cm.Position(), cm.Right(), cm.Up(), cm.Forward(), dir, distance,
        contact->second, &newPos, &newRight2, &newUp2, &newForward2, &newDir, &newFace, cur->TypeFilter());
    MoveMatrix hitLocal(newRight2, newUp2, newForward2, newPos);
    MoveMatrix hitWorld = cur->ToWorld(hitLocal);
    MoveVector end2 = newPos + newDir * distance;
    MoveVector newDirSaved = newDir;
    MoveVector worldEnd2 = cur->ToWorld(end2);
    int hit = 0;
    int i = 0;
    int newFaceSaved = newFace;

    for (i = 0; i < instances.count; i++) {
        if (contact->face == i)
            continue;
        MoveSurface *surf = instances.Get(i);
        if (surf == 0)
            continue;
        if (!IsNavigationSurfaceNear((void *)surf, *position) && !IsNavigationSurfaceNear((void *)surf, end)) {
            if (!surf->Changed())
                continue;
            if (!surf->WasNear(*position) && !surf->WasNear(end))
                continue;
        }

        MoveMatrix local = surf->Relative(frame);
        int hitFace;

        if (cur->Changed() || surf->Changed()) {
            // Something moved this frame: cast from where the body was relative to the surface before the move.
            MoveVector a = cm.Position();
            MoveVector b = local.Position();
            MoveVector prev = cur->ToWorldBefore(a);
            MoveVector before = surf->ToLocalBefore(prev);
            MoveVector mv = b - before;
            float length = mv.Normalize();
            hit = CastNavigationRay((NavMesh *)surf->GetSurface()->mesh, before, local.Right(), local.Up(), local.Forward(), mv, length,
                &newPos, &newRight2, &newUp2, &newForward2, &hitFace, surf->TypeFilter());
        }
        if (hit) {
            MoveContact c(i, hitFace);
            if (FlyThrough(&c)) {
                RUN_SURFACE_SCRIPT();
                RESTORE_FIRST_MOVE();
                hit = 0;
            } else {
                cur = surf;
                newFace = hitFace;
            }
            break;
        }

        localEnd = surf->ToLocal(hitWorld.Position());
        {
            MoveVector d = localEnd - local.Position();
            float length = d.Normalize();
            hit = CastNavigationRay((NavMesh *)surf->GetSurface()->mesh, local.Position(), local.Right(), local.Up(), local.Forward(), d,
                length, &newPos, &newRight2, &newUp2, &newForward2, &hitFace, surf->TypeFilter());
        }
        if (hit) {
            MoveContact c(i, hitFace);
            if (FlyThrough(&c)) {
                RUN_SURFACE_SCRIPT();
                RESTORE_FIRST_MOVE();
                hit = 0;
            } else {
                cur = surf;
                newFace = hitFace;
                MoveFrame m(newRight2, newUp2, newForward2, newPos);
                localEnd = surf->ToLocal(end);
                dir = localEnd - local.Position();
                dir.Normalize();
                float d = (local.Position() - newPos).Length();
                if (d < distance) {
                    int ok = surf->GetSurface()->mesh->Move(newPos, newRight2, newUp2, newForward2, dir, distance - d,
                        newFace, &newPos, &newRight2, &newUp2, &newForward2, &newDir, &newFace, surf->TypeFilter());
                    if (ok) {
                        if (movedOk == 0) {
                            movedOk = 1;
                            hit = 0;
                            contactFace = i;
                        } else {
                            m.SetRows(newRight2, newUp2, newForward2, newPos);
                            MoveMatrix mw = surf->ToWorld(m);
                            MoveVector delta = mw.Position() - hitWorld.Position();
                            float along = Dot(hitWorld.Up(), delta);
                            float back = Dot(delta * -1.0f, mw.Up());
                            if (along <= 0.0f && back > 0.0f) {
                                RESTORE_FIRST_MOVE();
                            } else {
                                movedOk = 1;
                                contactFace = i;
                            }
                            hit = 0;
                        }
                    } else {
                        if (movedOk) {
                            RESTORE_FIRST_MOVE();
                        }
                        hit = 0;
                    }
                } else {
                    MoveVector w = surf->ToLocal(end);
                    newDir = w - local.Position();
                    newDir.Normalize();
                }
            }
            break;
        }

        if (movedOk == 0) {
            // Nothing so far: look again from just above the surface.
            local = surf->Relative(hitWorld);
            localEnd = surf->ToLocal(worldEnd2);
            local.SetTranslation(local.Position() + local.Up() * 0.05f);
            localEnd = localEnd - local.Up() * 0.05f;
            MoveVector d = localEnd - local.Position();
            float length = dir.Normalize();     // sic: the original normalizes dir, not d
            hit = CastNavigationRay((NavMesh *)surf->GetSurface()->mesh, local.Position(), local.Right(), local.Up(), local.Forward(), d,
                length, &newPos, &newRight2, &newUp2, &newForward2, &hitFace, surf->TypeFilter());
        }
        if (hit) {
            cur = surf;
            newFace = hitFace;
            break;
        }
    }

    if (movedOk == 0 && hit == 0)
        return 0;

    MoveFrame outFrame(newRight2, newUp2, newForward2, newPos);
    MoveMatrix world = cur->ToWorld(outFrame);
    *newPosition = world.Position();
    *newRight = world.Right();
    *newUp = world.Up();
    *newForward = world.Forward();
    if (hit) {
        *newDirection = *direction;
        newContact->face = i;
        newContact->second = newFace;
        newContact->matrix = outFrame;
        return hit;
    }
    MoveVector t = newPos + newDir;
    MoveVector worldPoint = cur->ToWorld(t);
    MoveVector nd = worldPoint - *newPosition;
    nd.Normalize();
    *newDirection = nd;
    newContact->face = contactFace;
    newContact->second = newFace;
    newContact->matrix = outFrame;
    return movedOk;
}
