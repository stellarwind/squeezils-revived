// Simulation: see Simulation.h. Reconstructed from SQZ.EXE 0x412da0 and 0x414010-0x4142b0.
#include "Simulation.h"
#include "VirtualCall.h"

static_assert(offsetof(CreatureTable, values) == 0x18, "CreatureTable layout");
static_assert(offsetof(LocationSimulation, ids) == 0xc, "LocationSimulation layout");

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
