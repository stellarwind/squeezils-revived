// Simulation: the game world being run (a region on the map or a place indoors). Reconstructed from SQZ.EXE 0x412da0 and
// 0x414010-0x4142b0. Only the part that keeps the creatures is done so far.
//
// Every creature in a simulation has a whole number id, and a record (Creature) that holds two objects: its actor (the thing
// that is drawn and moved) and its soul (the thing that decides what it does). The simulation keeps the records in a table by
// id, and the ids in the order they were added. Props, scripts and the camera find the player and the other creatures by asking
// the current simulation (the global at 0x4badbc) for the actor or soul of an id; the player is id 0.
#pragma once
#include "Container.h"
#include "VirtualCall.h"

// @0049d5c4 vtable. The record is made and freed by the game's own heap; the vtable pointer is kept by hand (see VirtualCall.h).
struct Creature {
    GAME_HEAP_OPERATORS
    void **vtbl;      // 0x0
    void  *actor;     // 0x4 virtual slot 0 destroys it
    void  *soul;      // 0x8
};
static_assert(sizeof(Creature) == 0xc, "Creature layout");

// The BodySimSoulTable (vtable 0x49d5f4): creature records by id. Its functions are the original's, called through the table in
// the object (the Table template of Container.h); only Put is reconstructed.
struct CreatureTable {
    void **vtbl;                  // 0x00
    Set<int, 10> keys;            // 0x04 the ids, sorted
    Array<Creature *> values;     // 0x18 values.data[i] belongs to keys.data[i]

    int  Contains(int id) { return CallVirtual<int>(this, 1, &id); }
    int  IndexOf(int id)  { return CallVirtual<int>(this, 2, &id); }       // -1 when missing
    void RemoveAt(int i)  { CallVirtual<void>(this, 3, i); }
    void Insert(int id, Creature *creature) { CallVirtual<int>(this, 0, &id, &creature, -1); }   // adds or replaces

    // @00412da0 Files the actor and soul under the id; a creature already there is destroyed first (its actor, its soul, its record).
    void Put(int id, void *actor, void *soul);
};

struct Simulation {
    GAME_HEAP_OPERATORS
    void **vtbl;      // 0x0 @0049d644 (the base class has nothing but the destructor)

    // @00414280 slot 0
    Simulation *Destroy(int flags);
};

struct LocationSimulation : Simulation {
    CreatureTable *creatures;   // 0x04
    int            lastId;      // 0x08 the last id NewId handed out
    Array<int>     ids;         // 0x0c the ids in the order they were added (the original list class has a BufferObject on the end)
    void          *idsBufferObject;   // 0x1c
    int            active;      // 0x20 set to 1 by the constructor

    LocationSimulation *Construct();            // @00412e80 (the id list is made by the original)
    void  Destruct();                           // @00412f20
    LocationSimulation *DestroyAndFree(int flags);   // @00412f00 slot 0 (bit 0 of flags: free the memory)
    void  Init();                               // @00412f90 makes the creature table, ids start at 10; drains the key and mouse messages
    void  ClearCreatures();                     // @00413060 DestroyCreatures, then whatever creatures are left in the table

    void  Clear();                              // @004130b0 slot 2: frees the creature table (not the creatures), the scene and the parsed ini files
    void  UpdateSouls();                        // @00413f80 slot 3: the souls decide (the player's, then the others, the camera's last), then every actor moves; then the creatures marked with AddId are destroyed
    void  RunActors();                          // @00414010 slot 4: the actors' virtual slot 2, one after another (not identified)
    void *GetActor(int id);                     // @00414050 NULL when there is no creature with the id
    void *GetSoul(int id);                      // @00414080 NULL when there is no creature with the id
    int   FindIdBySoul(void *soul);             // @004140b0 -1 when there is none
    int   FindIdByActor(void *actor);           // @00414100 -1 when there is none
    CreatureTable *GetCreatures() { return creatures; }   // @00414150
    void  AddCreature(int id, void *actor, void *soul);   // @00414160
    void  AddId(int id);                        // @00414180 notes the id as the newest
    void  DestroyCreatures();                   // @004141a0 destroys every creature in the id list and empties the list
    int   NewId();                              // @00414250 the next id above the last one handed out that no creature has

    // @00413280 slot 8 (Region and Place use it as is): one frame of creature against creature collisions, then the creatures
    // marked with AddId are destroyed. Slot 12 (not reconstructed; Region and Place have it) tells the body's own velocity.
    void  CollideCreatures();
    int   IsMarked(int id) { return CallVirtual<int>(&ids, 6, &id); }   // the id list is a set (vtable @004a009c); slot 6 is Contains
};

// @004142b0 an empty record (no actor, no soul)
Creature *NewCreature();

// @00413100 Whether the point is within sqrt(maxDistSq) of the line from origin towards end, measured only over the first unit
// of length (the direction is made length 1 and the foot of the perpendicular must lie 0 to 1 along it).
struct Vector3f;
int PointNearSegment(const Vector3f *point, const Vector3f *origin, const Vector3f *end, float maxDistSq);

// @0047d680 Where the lines p1-p2 and p3-p4 pass closest (not parallel; 0 when they are): *s is that point as a part of p1-p2 (0 at
// p1, 1 at p2), *t the same for p3-p4.
int ClosestLineParams(const Vector3f *p1, const Vector3f *p2, const Vector3f *p3, const Vector3f *p4, float *s, float *t);

// @0047d940 The point on the line through a and b that is nearest to point; returns out.
Vector3f *NearestPointOnLine(Vector3f *out, const Vector3f *point, const Vector3f *a, const Vector3f *b);
