// Simulation: see Simulation.h. Reconstructed from SQZ.EXE 0x412da0 and 0x414010-0x4142b0.
#include "Simulation.h"
#include "Original.h"
#include "Vector3f.h"
#include "VirtualCall.h"
#include "Body.h"
#include "World.h"
#include "IniFile.h"
#include <string.h>
#include <math.h>

static_assert(offsetof(CreatureTable, values) == 0x18, "CreatureTable layout");
static_assert(offsetof(LocationSimulation, ids) == 0xc, "LocationSimulation layout");
static_assert(offsetof(LocationSimulation, active) == 0x20, "LocationSimulation layout");

// Destroys one of a creature's objects (slot 0 with bit 0 set: free the memory too)
static void DestroyObject(void *object)
{
    if (object)
        CallVirtual<void>(object, 0, 1);
}

// @00412da0
void CreatureTable::Put(int id, void *actor, void *soul)
{
    if (Contains(id)) {
        Creature *old = values.At(IndexOf(id));
        DestroyObject(old->actor);
        DestroyObject(old->soul);
        delete old;
    }
    Creature *creature = new Creature;
    creature->vtbl = (void **)0x49d5c4;
    creature->actor = actor;
    creature->soul = soul;
    Insert(id, creature);
}

// @004142b0
Creature *NewCreature()
{
    Creature *creature = new Creature;
    creature->vtbl = (void **)0x49d5c4;
    creature->actor = NULL;
    creature->soul = NULL;
    return creature;
}

// @00414280
Simulation *Simulation::Destroy(int flags)
{
    vtbl = (void **)0x49d644;
    if (flags & 1)
        delete this;
    return this;
}

// @00414010 (the table is looked at again on every pass: an actor may change it)
void LocationSimulation::RunActors()
{
    for (int i = 0; i < creatures->keys.count; i++)
        CallVirtual<void>(creatures->values.At(i)->actor, 2);
}

// @00414050
void *LocationSimulation::GetActor(int id)
{
    int index = creatures->IndexOf(id);
    if (index == -1)
        return NULL;
    return creatures->values.At(index)->actor;
}

// @00414080
void *LocationSimulation::GetSoul(int id)
{
    int index = creatures->IndexOf(id);
    if (index == -1)
        return NULL;
    return creatures->values.At(index)->soul;
}

// @004140b0
int LocationSimulation::FindIdBySoul(void *soul)
{
    for (int i = 0; i < creatures->keys.count; i++) {
        if (creatures->values.At(i)->soul == soul)
            return creatures->keys.At(i);
    }
    return -1;
}

// @00414100
int LocationSimulation::FindIdByActor(void *actor)
{
    for (int i = 0; i < creatures->keys.count; i++) {
        if (creatures->values.At(i)->actor == actor)
            return creatures->keys.At(i);
    }
    return -1;
}

// @00414160
void LocationSimulation::AddCreature(int id, void *actor, void *soul)
{
    creatures->Put(id, actor, soul);
}

// @00414180
void LocationSimulation::AddId(int id)
{
    ids.Insert(id, -1);
}

// @004141a0
void LocationSimulation::DestroyCreatures()
{
    for (int i = 0; i < ids.count; i++) {
        int index = creatures->IndexOf(ids.At(i));
        if (index == -1)
            continue;
        Creature *creature = creatures->values.At(index);
        DestroyObject(creature->actor);
        DestroyObject(creature->soul);
        delete creature;
        creatures->RemoveAt(index);
    }
    for (int i = ids.count - 1; i >= 0; i--)
        ids.RemoveAt(i);
}

// @00414250 (the id counter itself is what the table is asked about)
int LocationSimulation::NewId()
{
    do {
        lastId++;
    } while (creatures->Contains(lastId));
    return lastId;
}

// @00412e80
LocationSimulation *LocationSimulation::Construct()
{
    vtbl = (void **)0x49d644;
    ConstructIdList(&ids);
    vtbl = (void **)0x49d60c;
    active = 1;
    return this;
}

// @00412f20 (it ends as a plain Simulation again)
void LocationSimulation::Destruct()
{
    vtbl = (void **)0x49d60c;
    DestructIdList(&ids);
    vtbl = (void **)0x49d644;
}

// @00412f00
LocationSimulation *LocationSimulation::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        delete this;
    return this;
}

// @00412f90
void LocationSimulation::Init()
{
    CreatureTable *table = (CreatureTable *)Operator_New(0x30);
    if (table) {
        ConstructCreatureTable(table);
        table->vtbl = (void **)0x49d5f4;
        *(void **)((char *)table + 0x2c) = (void *)0x49d5dc;     // the table's BufferObject part
    }
    lastId = 10;
    creatures = table;
    FlushInputMessages(GetApplication());
}

// @00413060
void LocationSimulation::ClearCreatures()
{
    DestroyCreatures();
    for (int i = creatures->keys.count - 1; i >= 0; i--) {
        Creature *creature = creatures->values.At(i);
        DestroyObject(creature->actor);
        DestroyObject(creature->soul);
        delete creature;
        creatures->RemoveAt(i);
    }
}

// @00413100 (a point against a line from origin towards end: how far along it the nearest point is, and how far away it is)
int PointNearSegment(const Vector3f *point, const Vector3f *origin, const Vector3f *end, float maxDistSq)
{
    Vector3f direction = *end - *origin;
    Normalize(direction);
    float along = ((point->x * direction.x + point->y * direction.y + point->z * direction.z) -
                   (origin->x * direction.x + origin->y * direction.y + origin->z * direction.z)) /
                  (direction.y * direction.y + direction.z * direction.z + direction.x * direction.x);
    if (along < 0.0f || along > 1.0f)
        return 0;
    Vector3f nearest = *origin + direction * along;
    Vector3f offset = *point - nearest;
    return (offset.x * offset.x + offset.y * offset.y + offset.z * offset.z) < maxDistSq;
}

// @0047d680
int ClosestLineParams(const Vector3f *p1, const Vector3f *p2, const Vector3f *p3, const Vector3f *p4, float *s, float *t)
{
    Vector3f a = *p2 - *p1;
    float lengthA = Normalize(a);
    Vector3f b = *p4 - *p3;
    float lengthB = Normalize(b);
    Vector3f c = Cross(a, b);
    float cLength = Length(c);
    float cSquared = cLength * cLength;
    if (fabsf(cSquared) < 1e-8f)
        return 0;
    Vector3f w = *p3 - *p1;
    Vector3f bc = Cross(b, c);
    Vector3f ac = Cross(a, c);
    *s = (w.x * bc.x + w.y * bc.y + w.z * bc.z) / (lengthA * cSquared);
    *t = (w.x * ac.x + w.y * ac.y + w.z * ac.z) / (lengthB * cSquared);
    return 1;
}

// @0047d940
Vector3f *NearestPointOnLine(Vector3f *out, const Vector3f *point, const Vector3f *a, const Vector3f *b)
{
    Vector3f d = *b - *a;
    Normalize(d);
    float pointAlong = point->z * d.z + point->y * d.y + point->x * d.x;
    float aAlong = a->z * d.z + a->y * d.y + a->x * d.x;
    float t = -((aAlong - pointAlong) / (d.y * d.y + d.z * d.z + d.x * d.x));
    *out = *a + d * t;
    return out;
}

static float CollisionRadius(void *body)  { return *(float *)((char *)body + 0xb8); }   // the first field after Body (see GoodieBody, CameraBody, SqueezilBody)
static bool  NonNegative(float v)         { unsigned bits; memcpy(&bits, &v, 4); return bits <= 0x80000000u; }
static bool  InUnitRange(float v)         { int bits; memcpy(&bits, &v, 4); return NonNegative(v) && bits <= 0x3f800000; }
static float LengthSquared(const Vector3f &v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

enum { SOUL_IS_ACTIVE = 2, SOUL_COLLIDES = 3, SOUL_TOUCH = 4, SOUL_NOTE_NEIGHBOUR = 6, SOUL_BEGIN_FRAME = 7, SIM_BODY_VELOCITY = 12 };

// @00413280 Every pair of creatures that have a collision radius and an active soul is tested if at least one of them moves
// (a body's motion is its facing times its speed, plus the velocity the simulation reports for it). Each frame's motion is
// swept as a segment from the body's centre (its position raised by the radius along its up row); the creatures touch when
// the segments pass within the sum of the radii. Of the two, the one with the greater speed has its soul touched, with the
// other's id. Every pair is first made known to both souls (slot 6), a pair with an id marked by AddId is left out, and
// the marked creatures are destroyed at the end.
void LocationSimulation::CollideCreatures()
{
    if (!active)
        return;
    int count = creatures->keys.count;
    for (int i = 0; i < count; i++)
        CallVirtual<void>(creatures->values.At(i)->soul, SOUL_BEGIN_FRAME);

    for (int i = 0; i < count - 1; i++) {
        Creature *creatureA = creatures->values.At(i);
        void *a = creatureA->actor;
        float radiusA = CollisionRadius(a);
        if (radiusA == 0.0f || !CallVirtual<int>(creatureA->soul, SOUL_IS_ACTIVE))
            continue;
        float speedA = CallVirtual<float>(a, BODY_GET_FACING_VALUE);
        Vector3f velocityA;
        CallVirtual<void>(this, SIM_BODY_VELOCITY, &velocityA, a);
        bool movingA = !(fabsf(speedA) <= 0.01f && fabsf(fabsf(velocityA.z) + fabsf(velocityA.y) + fabsf(velocityA.x)) <= 0.01f);
        if (!movingA)
            speedA = 0.0f;

        for (int j = i + 1; j < count; j++) {
            Creature *creatureB = creatures->values.At(j);
            void *b = creatureB->actor;
            float radiusB = CollisionRadius(b);
            if (radiusB == 0.0f || !CallVirtual<int>(creatureB->soul, SOUL_IS_ACTIVE))
                continue;
            int idA = creatures->keys.At(i);
            if (IsMarked(idA))
                continue;
            int idB = creatures->keys.At(j);
            if (IsMarked(idB))
                continue;
            float speedB = CallVirtual<float>(b, BODY_GET_FACING_VALUE);
            Vector3f velocityB;
            CallVirtual<void>(this, SIM_BODY_VELOCITY, &velocityB, b);
            bool movingB = !(fabsf(speedB) <= 0.01f && fabsf(fabsf(velocityB.z) + fabsf(velocityB.y) + fabsf(velocityB.x)) <= 0.01f);
            if (!movingB)
                speedB = 0.0f;
            if (!movingA && !movingB)
                continue;

            float *matrixA = CallVirtual<float *>(a, BODY_GET_MATRIX);
            float *matrixB = CallVirtual<float *>(b, BODY_GET_MATRIX);
            Vector3f positionA(matrixA[12], matrixA[13], matrixA[14]);
            Vector3f positionB(matrixB[12], matrixB[13], matrixB[14]);
            Vector3f apart = positionB - positionA;
            float distanceSquared = LengthSquared(apart);

            CallVirtual<void>(creatureA->soul, SOUL_NOTE_NEIGHBOUR, &idB);
            CallVirtual<void>(creatureB->soul, SOUL_NOTE_NEIGHBOUR, &idA);

            float seconds = (float)FrameMilliseconds() * 0.001f;
            Vector3f motionA = *CallVirtual<Vector3f *>(a, BODY_GET_FACING) * speedA + velocityA;
            Vector3f motionB = *CallVirtual<Vector3f *>(b, BODY_GET_FACING) * speedB + velocityB;
            Vector3f relative = motionA - motionB;
            if (apart.y * relative.y + apart.z * relative.z + apart.x * relative.x < 0.0f)
                continue;

            float reach = radiusA + radiusB;
            float reachSquared = reach * reach;
            int hit;
            if (!(Length(relative) * seconds > reach)) {
                hit = reachSquared > distanceSquared;
            } else {
                if (!CallVirtual<int>(creatureA->soul, SOUL_COLLIDES) || !CallVirtual<int>(creatureB->soul, SOUL_COLLIDES))
                    continue;
                Vector3f startA = positionA + Vector3f(matrixA[4], matrixA[5], matrixA[6]) * radiusA;
                Vector3f startB = positionB + Vector3f(matrixB[4], matrixB[5], matrixB[6]) * radiusB;
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
                    Vector3f nearest;
                    NearestPointOnLine(&nearest, &startA, &startB, &endB);
                    hit = LengthSquared(startA - nearest) < reachSquared;
                } else {
                    bool inA = InUnitRange(sA), inB = InUnitRange(sB);
                    if (inA && inB) {
                        Vector3f pointA = startA + travelA * sA;
                        Vector3f pointB = startB + travelB * sB;
                        hit = LengthSquared(pointB - pointA) < reachSquared;
                    } else if (inA) {
                        const Vector3f &clampB = NonNegative(sB) ? endB : startB;
                        Vector3f pointA = startA + travelA * sA;
                        hit = LengthSquared(pointA - clampB) < reachSquared;
                    } else if (inB) {
                        const Vector3f &clampA = NonNegative(sA) ? endA : startA;
                        Vector3f pointB = startB + travelB * sB;
                        hit = LengthSquared(pointB - clampA) < reachSquared;
                    } else {
                        hit = PointNearSegment(NonNegative(sA) ? &endA : &startA, &startB, &endB, reachSquared);
                        if (!hit)
                            hit = PointNearSegment(NonNegative(sB) ? &endB : &startB, &startA, &endA, reachSquared);
                    }
                }
            }
            if (hit) {
                if (speedB < speedA)
                    CallVirtual<void>(creatureA->soul, SOUL_TOUCH, idB);
                else
                    CallVirtual<void>(creatureB->soul, SOUL_TOUCH, idA);
            }
        }
    }
    DestroyCreatures();
}

// @004130b0 (the table is freed with the original's destructor: it deletes the creature records, which no longer hold anything
// by then; the pointer is left as it was)
void LocationSimulation::Clear()
{
    if (creatures) {
        DestructCreatureTable(creatures);
        Operator_Delete(creatures);
    }
    GetWorld()->DestroyScene();
    IniCloseAll();
}

// @00413f80 The camera is entry 1 of the table: its soul is updated after all the others.
void LocationSimulation::UpdateSouls()
{
    CallVirtual<void>(creatures->values.At(0)->soul, 5);
    for (int i = 2; i < creatures->keys.count; i++)
        CallVirtual<void>(creatures->values.At(i)->soul, 5);
    CallVirtual<void>(creatures->values.At(1)->soul, 5);
    for (int i = 0; i < creatures->keys.count; i++)
        CallVirtual<void>(creatures->values.At(i)->actor, 1);
    DestroyCreatures();
}
