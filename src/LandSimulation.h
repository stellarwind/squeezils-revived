// LandSimulation: the map of the whole game (the regions and what the player carries between them). Reconstructed from
// SQZ.EXE 0x41e310-0x41ea60. One exists for the whole run (the global at 0x4badb8, made by the application start at
// 0x4203f9; 0x420680 returns it). It is not the world being played (that is the RegionSimulation or PlaceSimulation in the
// global at 0x4badbc); it owns the region names from the [RegionList] section of sqz.ini and the "possessions" (inventory)
// of the characters, which it lends to the simulation that is running. Key 0 of the table is the player.
//
// It is a Simulation (slot list below) and also a BufferObject (+4), so the whole map can go into a save file: the table
// and the list of lent-out ids are what a save stores.
#pragma once
#include "Simulation.h"
#include "SqueezilSoul.h"      // Possessions

// The table of the possessions by character id (BufferObjectTable<int, Character *, ...>, vtables 0x49d87c and 0x49d864,
// 0x30 bytes). The layout is CreatureTable's (Table of a Set of keys and a PrimitiveArray of values); the original makes and
// frees it (0x4650b0, 0x4651d0).
struct PossessionsTable {
    void **vtbl;                  // 0x00 @0049d87c
    Set<int, 5> keys;             // 0x04 the ids, sorted
    Array<Possessions *> values;  // 0x18
    // 0x28 the values' BufferObject, 0x2c the table's BufferObject (@0049d864)
};

class IniReader;

struct LandSimulation : Simulation, BufferObject {
    PossessionsTable *table;        // 0x08 the possessions by character id
    void             *lentIds;      // 0x0c Set<int, 10> (vtable @004a0064 is its array part): the ids whose possessions are lent out
    char            **names;        // 0x10 the region names, one 80 byte buffer each; counted from 1 (names[0] is never used)
    int               regionCount;  // 0x14

    LandSimulation *Construct();                    // @0041e310 (called by the application start; 0x18 bytes)
    void  Destruct();                               // @0041e3a0 frees the table, the id list and the names
    LandSimulation *DestroyAndFree(int flags);      // @0041e380 slot 0
    void  Init();                                   // @0041e490 slot 1: the table with the player's possessions, the names from [RegionList]
    void  Clear();                                  // @0041e700 slot 2: destroys the possessions that are not lent out, frees table and ids
    // slots 3, 4, 5 and 6 (0x41e800, 0x41ea40, 0x41e7e0, 0x41e7f0) do nothing
    int   CharacterCount();                         // @0041e810 slot 7
    PossessionsTable *GetTable();                   // @0041e820 slot 8
    int   IsLentOut(int id);                        // @0041e830 slot 9 (names are inferred from the use: the callers lend a character's
    Possessions *Lend(int id);                      // @0041e850 slot 10  possessions to a running simulation with slot 10 and take them
    void  Return(int id, Possessions *possessions); // @0041e880 slot 11  back with slot 11)
    const char *GetPlaceName(int index);            // @0041e8c0 the region's name, index from 1
    int   PlaceCompleted(int region, int place);    // @0041e9b0 ret 8: the player has finished that place (the pairs of "PlacesCompleted" in the inventory)

    // BufferObject (vtable @0049d814, the pointer to this part is the object + 4)
    // slot 0 @0041ea50 returns "LandSimulation", slot 1 @0041ea60 makes an empty one, slot 2 @0041e8d0 writes the table and the id
    // list, slot 3 @0041e910 replaces them with what it reads
    static const char *TypeName();
    static BufferObject *Create();
    void  Write(void *stream);
    void  Read(void *stream);
};
static_assert(offsetof(LandSimulation, table) == 0x8, "LandSimulation layout");
static_assert(sizeof(LandSimulation) == 0x18, "LandSimulation layout");

// @004488d0 A fresh set of possessions (8 bytes, vtable @0049ec8c) with an empty inventory (0x70 bytes, @00415ef0)
Possessions *NewPossessions();

// @00420680 the global at 0x4badb8
inline LandSimulation *GetLand() { return *(LandSimulation **)0x4badb8; }
