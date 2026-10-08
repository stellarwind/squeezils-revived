// RegionSimulation: one region of the map being played (the outdoor world with its tree of places). Reconstructed from SQZ.EXE
// 0x416c60-0x418a60 (RTTI class RegionSimulation, vtable 0x49d70c). The simulation in the global at 0x4badbc is one of these
// while the player walks about a region, and a PlaceSimulation (PlaceSimulation.h) while they are inside a place.
//
// A region is a folder of the game (named by the LandSimulation's region list) with a region\region.ini. Its [LocationList] gives
// the spots of the map: places the player can enter (shown as trees), portals to other regions, nav spots, key spots and relic
// spots (each a PlaceDescriptor). The player starts at the spot `location` (the spot they last left).
//
// Slot list: 0 Destroy, 1 Init (loads the region), 2 Clear, 3 5 6 7 4 8 are LocationSimulation's, 9 CanSave (1), 10 Load,
// 11 Save, 12 Velocity (zero).
#pragma once
#include "Simulation.h"
#include "Float3.h"
#include "Container.h"
#include "IniReader.h"

struct Prop;
struct Inventory;
struct IDirect3DRMFrame;

// One spot of the map (a [LocationList] entry of region.ini). The class has no RTTI name in the game; the name of its table
// (Table<int, PlaceDescriptor *, ...>) is the only one it has.
struct PlaceDescriptor {
    GAME_HEAP_OPERATORS                 // the game frees these with its own operator delete
    enum Kind { PLACE = 0, NAV_SPOT = 1, PORTAL = 2, KEY_OR_RELIC = 3 };

    Float3   position;          // 0x00 "Location": where the spot is on the map
    IntList *destinations;      // 0x0c "Destination" (places and portals: where it leads; an empty list for the other kinds)
    IntList *links;             // 0x10 "Links": the spots one can walk on to from here
    String   function;          // 0x14 "Function": the script function named by the spot
    Float3   propPosition;      // 0x18 "PropPosition": where its prop (the tree) stands
    IDirect3DRMFrame *frame;    // 0x24 the frame the prop is built under (places only)
    Float3   propScale;         // 0x28 "PropScale"
    Prop    *prop;              // 0x34 the shown prop (an instance of one of the four tree sprites)
    int      state;             // 0x38 which tree sprite is shown (see LocationState): 0 and 1 for a finished place, 2 and 3 for an open one
    int      kind;              // 0x3c Kind
    int      id;                // 0x40 the spot's number in the list

    PlaceDescriptor *Init();                // @00416c60 position, prop position, prop scale, function empty, no lists, no prop
    void             Destruct();            // @00416d40 frees the two lists and destroys the shown prop (not the frame)
};
static_assert(sizeof(PlaceDescriptor) == 0x44, "PlaceDescriptor layout");

// Table<int, PlaceDescriptor *, Array<PlaceDescriptor *>, 8> (vtable @004a097c; made by @004690c0, 0x28 bytes). Same layout as
// the creature table minus the save/load parts.
struct PlaceTable {
    void **vtbl;                                  // 0x00
    Set<int, 8> keys;                             // 0x04 the spot numbers, sorted
    Array<PlaceDescriptor *> values;              // 0x18

    int  Contains(int id) { return CallVirtual<int>(this, 1, &id); }
    int  IndexOf(int id)  { return CallVirtual<int>(&keys, 7, &id); }      // -1 when missing
    void Insert(int id, PlaceDescriptor *place) { CallVirtual<int>(this, 0, &id, &place, -1); }   // adds or replaces
    PlaceDescriptor *Get(int id) { return values.At(IndexOf(id)); }       // @00469a70, not checked: callers test Contains first
};
static_assert(sizeof(PlaceTable) == 0x28, "PlaceTable layout");

struct RegionSimulation : LocationSimulation {
    int         region;         // 0x24 the region's number (from 1; the LandSimulation names it)
    int         location;       // 0x28 the spot the player stands at (a PlaceDescriptor number)
    int         placeCount;     // 0x2c how many spots ReadLocation has read
    PlaceTable  places;         // 0x30 the spots by number
    Prop       *unblurredTree;    // 0x58 the four tree sprites (templates found by name, keys of region.ini [Defaults]): UnblurredTreeSprite
    Prop       *unblurredNutTree; // 0x5c UnblurredTreeSpriteNut
    Prop       *blurredTree;      // 0x60 BlurredTreeSprite
    Prop       *blurredNutTree;   // 0x64 BlurredTreeSpriteNut (state 2 is shown with this one, state 3 with UnblurredTreeSpriteNut)
    // total 0x68

    RegionSimulation *Construct(int region, int location);   // @00416e50 ret 8
    void  Destruct();                                        // @00416ee0 back to a LocationSimulation (the table goes with it)
    RegionSimulation *DestroyAndFree(int flags);             // @00418a40 slot 0
    void  Init();                                            // @00416f50 slot 1: loads the region (see the function for the order)
    void  Clear();                                           // @00417600 slot 2: stops the sound, frees the props, scripts and spots
    int   CanSave() { return 1; }                            // @004189a0 slot 9
    int   Load(const char *path);                            // @00418680 slot 10 (ret 4): a saved game, replaces this simulation; 0 on failure
    int   Save(const char *path);                            // @004188c0 slot 11 (ret 4): 0 on failure
    Float3 *Velocity(Float3 *out, void *body);                         // @004189b0 slot 12 (ret 8): zero

    void  ReadLocations(IniReader *ini);                     // @00417680 ret 4: every [LocationList] entry, then drops links to missing spots
    PlaceDescriptor *ReadLocation(IniReader *ini, const char *name, int id);   // @004178e0 ret 0xc
    void  RegisterPlace(PlaceDescriptor *place, int id);     // @004180e0 ret 8: its state and its tree
    void  SetLocation(int location);                         // @00418170 ret 4: moves the player's spot; picks up keys and relics there
    int   PlaceCount()            { return placeCount; }     // @00418110
    int   HasPlace(int id)        { return places.Contains(id); }   // @00418120
    PlaceDescriptor *GetPlace(int id) { return places.Get(id); }   // @00418140
    int   GetLocation()           { return location; }       // @00418150
    int   GetRegion()             { return region; }         // @00418160
    int   LocationState(PlaceDescriptor *place, int id);     // @00418590 ret 8: 0 finished, 1 finished and next to the player, 2 open, 3 open and next to the player
    void  ShowPlaceProp(PlaceDescriptor *place, int state);  // @004183e0 ret 8: swaps the tree sprite of a place for the one of the state
};
static_assert(sizeof(RegionSimulation) == 0x68, "RegionSimulation layout");

// The inventory's raw flag tables (see QuestItems.cpp and CompassRuler.cpp): three key piece flags per region from +4, one relic
// flag per region from +0x54.
inline int *InventoryKeyFlags(Inventory *inv)   { return (int *)((char *)inv + 0x4); }
inline int *InventoryRelicFlags(Inventory *inv) { return (int *)((char *)inv + 0x54); }
inline int  RelicFlag(Inventory *inv, int region) { return InventoryRelicFlags(inv)[region]; }
// @00416580 all three key pieces of the region found
inline int  KeysFound(Inventory *inv, int region)
{
    for (int i = 0; i < 3; i++)
        if (!InventoryKeyFlags(inv)[region * 3 + i])
            return 0;
    return 1;
}
// @004165b0 the relics of regions 1 to 5 found
inline int  InventoryAllRelics(Inventory *inv)
{
    for (int region = 1; region <= 5; region++)
        if (!RelicFlag(inv, region))
            return 0;
    return 1;
}
