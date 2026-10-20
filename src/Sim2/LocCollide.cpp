// LocationSimulation slot 8 (SQZ.EXE 0x413280): one frame of creature-against-creature collisions.
// Differential regression against original machine code: tools/flagsearch/test_collision.sh.
#include "Vector3f/Vector3f.h"
#include "GameClock/GameClock.h"
#include "Globals/Globals.h"
#include <math.h>
#include <string.h>
struct Soul {
    virtual void v0(); virtual void v1();
    virtual int IsActive();                              // slot 2
    virtual int Collides();                              // slot 3
    virtual void Touch(int other);                       // slot 4
    virtual void v5();
    virtual void NoteNeighbour(int *other);              // slot 6
    virtual void BeginFrame();                           // slot 7
};
struct Body {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual float *GetMatrix();                          // slot 3
    virtual Vector3f *GetFacing();                       // slot 4
    virtual float GetFacingValue();                      // slot 5
    char pad[0xb8 - 4 - 4 * 6];
};
struct Creature { void *vtbl; Body *actor; Soul *soul; };
template <class T> struct Arr {
    virtual T Get(int i);
    int count, cap;
    T *data;
    T At(int i) { return data[i]; }
    T AtU(unsigned i) { return data[i]; }
};
struct CreatureTable {
    void *vtbl;
    Arr<int> keys;                                       // 0x04 (a Set; the id list)
    char pad[0x18 - 0x04 - 0x10];
    Arr<Creature *> values;                              // 0x18
};
struct IdSet { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
               virtual int Contains(const int &id); };   // slot 6
int __stdcall PointNearSegment(const Vector3f *point, const Vector3f *origin, const Vector3f *end, float maxDistSq);   // 0x413100
int ClosestLineParams(const Vector3f *p1, const Vector3f *p2, const Vector3f *p3, const Vector3f *p4, float *s, float *t);   // 0x47d680
Vector3f ClosestOnLine(const Vector3f &point, const Vector3f &a, const Vector3f &b);   // 0x47d940, hidden result pointer
inline bool NonNegative(float v) { unsigned bits = *(unsigned *)&v; return bits <= 0x80000000u; }
inline bool InUnitRange(float v) { int bits = *(int *)&v; return NonNegative(v) && bits <= 0x3f800000; }

struct CollisionNeighbour {
    int id;
    Vector3f position;
    float radius;
    int active;
    CollisionNeighbour() : id(-1), active(1) {}
    ~CollisionNeighbour();                              // 0x4044c0
};

struct LocationSimulation {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual Vector3f BodyVelocity(Body *b);              // slot 12 (ret 8)
    CreatureTable *creatures;
    int lastId;
    IdSet ids;
    char pad[0x20 - 0x0c - 4];
    int active;
    int IsMarked(int id) { return ids.Contains(id); }
    void DestroyCreatures();                             // 0x4141a0
    void CollideCreatures();
};

void LocationSimulation::CollideCreatures()
{
    if (!active)
        return;
    int count = creatures->keys.count;
    if (count > 0) {
        int n = count;
        int k = 0;
        do {
            creatures->values.At(k)->soul->BeginFrame();
            k++;
        } while (--n);
    }

    int i = 0;
    if (count - 1 > 0) do {
        Body *a = creatures->values.AtU(i)->actor;
        float radiusA = *(float *)((char *)a + 0xb8);
        if (radiusA == 0.0f || !creatures->values.AtU(i)->soul->IsActive())
            continue;
        float speedA = a->GetFacingValue();
        Vector3f velocityA = BodyVelocity(a);
        bool movingA = !(fabs(speedA) <= 0.01 && fabs(fabs(velocityA.z) + fabs(velocityA.y) + fabs(velocityA.x)) <= 0.01);
        if (!movingA)
            speedA = 0.0f;

        int j = i + 1;
        if (j < count) do {
            Body *b = creatures->values.AtU(j)->actor;
            float radiusB = *(float *)((char *)b + 0xb8);
            if (radiusB == 0.0f || !creatures->values.AtU(j)->soul->IsActive())
                continue;
            int idA = creatures->keys.Get(i);
            if (IsMarked(idA))
                continue;
            int idB = creatures->keys.Get(j);
            if (IsMarked(idB))
                continue;
            float speedB = b->GetFacingValue();
            Vector3f velocityB = BodyVelocity(b);
            bool movingB = !(fabs(speedB) <= 0.01 && fabs(fabs(velocityB.z) + fabs(velocityB.y) + fabs(velocityB.x)) <= 0.01);
            if (!movingB)
                speedB = 0.0f;
            if (!movingA && !movingB)
                continue;

            CollisionNeighbour neighbourB, neighbourA;
            neighbourB.position = *(Vector3f *)&b->GetMatrix()[12];
            neighbourA.position = *(Vector3f *)&a->GetMatrix()[12];
            Vector3f apart = *(Vector3f *)&b->GetMatrix()[12] - *(Vector3f *)&a->GetMatrix()[12];
            neighbourA.radius = neighbourB.radius = apart.y * apart.y + apart.z * apart.z + apart.x * apart.x;
            float distanceSquared = neighbourB.radius;

            Soul *soulA = creatures->values.AtU(i)->soul;
            Soul *soulB = creatures->values.AtU(j)->soul;
            neighbourB.id = creatures->keys.Get(j);
            neighbourA.id = creatures->keys.Get(i);
            soulA->NoteNeighbour(&neighbourB.id);
            soulB->NoteNeighbour(&neighbourA.id);

            float seconds = (float)(unsigned)((GameClock *)GetGameClock())->SimRate() * 0.001f;
            Vector3f motionA = *a->GetFacing() * speedA + velocityA;
            Vector3f motionB = *b->GetFacing() * speedB + velocityB;
            Vector3f relative = motionA - motionB;
            if (apart.y * relative.y + apart.z * relative.z + apart.x * relative.x < 0.0f)
                continue;

            float reach = radiusA + radiusB;
            float reachSquared = reach * reach;
            int hit;
            if (!(relative.Length() * seconds > reach)) {
                hit = reachSquared > distanceSquared;
            } else {
                if (!soulA->Collides() || !soulB->Collides())
                    continue;
                Vector3f startA, startB;
                startA = *(Vector3f *)&a->GetMatrix()[12] + *(Vector3f *)&a->GetMatrix()[4] * radiusA;
                Vector3f offsetB;
                offsetB = *(Vector3f *)&b->GetMatrix()[4] * radiusB;
                startB = *(Vector3f *)&b->GetMatrix()[12] + offsetB;
                Vector3f travelA = motionA * seconds;
                Vector3f travelB = motionB * seconds;
                Vector3f endA = startA + travelA;
                Vector3f endB = startB + travelB;
                float sA, sB;
                if (!movingA) {
                    hit = PointNearSegment(&startA, &startB, &endB, reachSquared);
                } else if (!movingB) {
                    hit = PointNearSegment(&startB, &startA, &endA, reachSquared);
                } else if (!ClosestLineParams(&startA, &endA, &startB, &endB, &sA, &sB)) {
                    Vector3f nearest = ClosestOnLine(startA, startB, endB);
                    hit = (startA - nearest).LengthSq() < reachSquared;
                } else {
                    bool inA = InUnitRange(sA), inB = InUnitRange(sB);
                    if (inA && inB) {
                        Vector3f pointA = startA + travelA * sA;
                        Vector3f pointB = startB + travelB * sB;
                        hit = (pointB - pointA).LengthSq() < reachSquared;
                    } else if (inA) {
                        const Vector3f &clampB = NonNegative(sB) ? endB : startB;
                        Vector3f pointA = startA + travelA * sA;
                        hit = (pointA - clampB).LengthSq() < reachSquared;
                    } else if (inB) {
                        const Vector3f &clampA = NonNegative(sA) ? endA : startA;
                        Vector3f pointB = startB + travelB * sB;
                        hit = (pointB - clampA).LengthSq() < reachSquared;
                    } else {
                        hit = PointNearSegment(NonNegative(sA) ? &endA : &startA, &startB, &endB, reachSquared);
                        if (!hit)
                            hit = PointNearSegment(NonNegative(sB) ? &endB : &startB, &startA, &endA, reachSquared);
                    }
                }
            }
            if (hit) {
                if (speedB < speedA)
                    soulA->Touch(creatures->keys.Get(j));
                else
                    soulB->Touch(creatures->keys.Get(i));
            }
        } while (count > ++j);
    } while (++i < count - 1);
    DestroyCreatures();
}
