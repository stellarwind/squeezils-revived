// Simulation: see Simulation.h. Reconstructed from SQZ.EXE 0x412da0 and 0x414010-0x4142b0.
#include "Simulation.h"
#include "Original.h"
#include "Vector3f.h"
#include "VirtualCall.h"

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
