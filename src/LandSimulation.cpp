// LandSimulation: see LandSimulation.h. Reconstructed from SQZ.EXE 0x41e310-0x41ea60.
#include "LandSimulation.h"
#include "IniReader.h"
#include "Original.h"
#include "VirtualCall.h"
#include <string.h>

static_assert(offsetof(PossessionsTable, values) == 0x18, "PossessionsTable layout");

enum { REGION_NAME_SIZE = 80 };

// @004650b0 and @0045eef0 construct the parts of a table and of an id list the original allocates (0x30 and 0x14 bytes); @004488d0 makes
// the possessions (vtable @0049ec8c, the inventory @00415ef0 in 0x70 bytes).
static PossessionsTable *NewPossessionsTable()
{
    PossessionsTable *t = (PossessionsTable *)Operator_New(0x30);
    if (t) {
        ConstructPossessionsTable(t);
        t->vtbl = (void **)0x49d87c;
        *(void ***)((char *)t + 0x2c) = (void **)0x49d864;
    }
    return t;
}

static void *NewIdList()
{
    void *ids = Operator_New(0x14);
    if (ids)
        ConstructIdList(ids);
    return ids;
}

Possessions *NewPossessions()
{
    Possessions *p = (Possessions *)Operator_New(8);
    if (p) {
        p->vtbl = (void **)0x49ec8c;
        Inventory *inv = (Inventory *)Operator_New(0x70);
        if (inv)
            InventoryConstruct(inv);
        p->inventory = inv;
    }
    return p;
}

// @0041e310 (the Simulation and BufferObject vtables are stored twice by the original, base class first)
LandSimulation *LandSimulation::Construct()
{
    Simulation::vtbl = (void **)0x49d644;
    table = NULL;
    Simulation::vtbl = (void **)0x49d82c;
    lentIds = NULL;
    BufferObject::vtbl = (void **)0x49d814;
    return this;
}

// @0041e3a0 (the original keeps the table's and the list's destructors as separate calls before the delete)
void LandSimulation::Destruct()
{
    Simulation::vtbl = (void **)0x49d82c;
    BufferObject::vtbl = (void **)0x49d814;
    if (table) {
        DestructPossessionsTable(table);
        Operator_Delete(table);
    }
    if (lentIds) {
        DestructIdList(lentIds);
        Operator_Delete(lentIds);
    }
    if (names) {
        // Both loops count from 1 (slot 0 is unused), but the delete loop reads the first name at index 1 after incrementing
        // the offset first, which is the same thing: names[1] .. names[regionCount].
        for (int i = 1; i <= regionCount; i++)
            Operator_Delete(names[i]);
        Operator_Delete(names);
    }
    Simulation::vtbl = (void **)0x49d644;      // ~Simulation (@00414270)
}

// @0041e380
LandSimulation *LandSimulation::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @0041e490
void LandSimulation::Init()
{
    table = NewPossessionsTable();
    lentIds = NewIdList();

    // key 0 is the player
    Possessions *player = NewPossessions();
    int playerKey = 0;
    CallVirtual<int>(table, 0, &playerKey, &player, -1);

    IniReader ini;
    ini.SetSection("RegionList");
    StringList *keys = ini.GetKeys();
    if (keys->count == 0) {
        GameLog("Error: no RegionList entry in sqz.ini!\n");
        return;
    }
    regionCount = keys->count;
    names = (char **)Operator_New(regionCount * 4 + 4);
    for (int i = 0; i < regionCount; i++) {
        names[i + 1] = (char *)Operator_New(REGION_NAME_SIZE);
        strcpy(names[i + 1], ini.GetString(keys->At(i).c_str()));    // no length check, as in the original
    }
    if (keys) {
        keys->~StringList();
        Operator_Delete(keys);
    }
}

// @0041e700
void LandSimulation::Clear()
{
    for (int i = 0; i < table->keys.count; i++) {
        Possessions *p = table->values.At(i);
        int id = table->keys.At(i);
        if (!IsLentOut(id) && p)
            CallVirtual<void>(p, 4, 1);          // destroy and free
    }
    if (table) {
        DestructPossessionsTable(table);
        Operator_Delete(table);
    }
    table = NULL;
    if (lentIds) {
        DestructIdList(lentIds);
        Operator_Delete(lentIds);
    }
    lentIds = NULL;
}

// @0041e810
int LandSimulation::CharacterCount() { return table->keys.count; }

// @0041e820
PossessionsTable *LandSimulation::GetTable() { return table; }

// @0041e830
int LandSimulation::IsLentOut(int id) { return CallVirtual<int>(lentIds, 6, &id); }

// @0041e850
Possessions *LandSimulation::Lend(int id)
{
    CallVirtual<int>(lentIds, 1, &id, -1);
    return table->values.At(CallVirtual<int>(&table->keys, 7, &id));
}

// @0041e880
void LandSimulation::Return(int id, Possessions *possessions)
{
    CallVirtual<void>(lentIds, 8, &id);
    int index = CallVirtual<int>(table, 2, &id);
    CallVirtual<int>(table, 0, &id, &possessions, index);
}

// @0041e8c0
const char *LandSimulation::GetPlaceName(int index) { return names[index]; }

// @0041ea50
const char *LandSimulation::TypeName() { return "LandSimulation"; }

// @0041ea60 (returns the BufferObject part)
BufferObject *LandSimulation::Create()
{
    LandSimulation *land = new LandSimulation;
    land->Construct();
    return land;
}

// @0041e8d0 (the table and the id list go to the stream as their BufferObject parts, the table's at +0x2c, the list's at +0x10)
void LandSimulation::Write(void *stream)
{
    BufferObject *tablePart = table ? (BufferObject *)((char *)table + 0x2c) : NULL;
    BufferObject *idsPart = lentIds ? (BufferObject *)((char *)lentIds + 0x10) : NULL;
    StreamWrite(StreamWrite(stream, tablePart), idsPart);
}

// @0041e910 (the old table and list are dropped first)
void LandSimulation::Read(void *stream)
{
    if (table) {
        DestructPossessionsTable(table);
        Operator_Delete(table);
    }
    if (lentIds) {
        DestructIdList(lentIds);
        Operator_Delete(lentIds);
    }
    StreamRead(StreamRead(stream, (BufferObject **)&table), (BufferObject **)&lentIds);
}

// @0041e9b0
int LandSimulation::PlaceCompleted(int region, int place)
{
    Inventory *inventory = Lend(0)->inventory;
    int count = InventoryCount(inventory, "PlacesCompleted");
    if (count <= 0)
        return 0;
    Array<int> *done = InventoryItems(Lend(0)->inventory, "PlacesCompleted");
    for (int i = 0; i < count; i += 2)
        if (done->At(i) == region && done->At(i + 1) == place)
            return 1;
    return 0;
}
