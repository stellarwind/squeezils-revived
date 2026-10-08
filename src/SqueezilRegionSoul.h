// SqueezilRegionSoul: the player's soul while they walk about a region (RegionSimulation.h). Reconstructed from SQZ.EXE
// 0x436710-0x438380 (RTTI class SqueezilRegionSoul, vtables 0x49e12c, 0x49e10c and 0x49e0e4).
//
// On the map the squeezil does not walk; it turns on the spot and, when the player pushes forward, flies along an arc to the spot
// (a PlaceDescriptor) it faces. Each frame the soul picks, among the spots the current one links to, the one that the squeezil points
// at most closely (within 20 degrees) and lets its tree blink; pushing forward starts the flight, which takes
// RegionFlightDuration frames. On arrival it makes the spot the current one and runs the spot's script function.
//
// Which neighbours can be aimed at: from a portal or a nav spot all of them; from a finished place or to a finished place all of
// them; to a place already visited, to a portal, to a key spot; from or to a key spot (300..399); and the ones LinkOpen allows
// (portals to the next region once the relics are in).
//
// The object is a SqueezilSoul with the input event target (vtable 0x49e0e4) at +0x80.
// InputEventTarget slots (the part at +0x80, `this` there is the soul + 0x80): 0 Enter (the action button: go into the place or
// through the portal), 1 OnEvent(what, text) (a typed spot number moves the squeezil there), 2 to 8 do nothing.
#pragma once
#include "SqueezilSoul.h"
#include "Simulation.h"
#include "Vector3f.h"

struct RegionSimulation;
struct PlaceDescriptor;

struct SqueezilRegionSoul : SqueezilSoul {
    void           **inputVtbl;         // 0x80 @0049e0e4 the InputEventTarget part
    int              flying;            // 0x84 set while a flight is on (the camera reads it)
    RegionSimulation *region;           // 0x88
    int              flightFrame;       // 0x8c frames flown so far
    int              location;          // 0x90 the spot the squeezil stands at (PlaceDescriptor number)
    int              target;            // 0x94 the neighbour aimed at (0: none); the flight's goal
    Vector3f         from;              // 0x98 where the flight started
    Vector3f         to;                // 0xa4 where it ends
    float            turnRate;          // 0xb0 degrees per frame, from the player's turning
    int              typing;            // 0xb4
    char             typed[8];          // 0xb8 the digits OnEvent was given
    int              typedLength;       // 0xc0
    char             unknownC4[0x20];   // 0xc4
    int              unknownE4;         // 0xe4 -1
    // total 0xe8

    SqueezilRegionSoul *Init(SqueezilBody *body, Possessions *possessions, RegionSimulation *region);   // @00436710 ret 0xc
    void  Destruct();                                       // @004369c0 (the part that is not the base class's)
    SqueezilRegionSoul *DestroyAndFree(int flags);          // @004369c0 slot 0
    void  StartFunction();                                  // @00436a60 runs the function named by the spot's "Function" key
    void  Enter();                                          // @00436af0 InputEventTarget slot 0 (this = soul + 0x80)
    void  OnEvent(int what, const char *text);              // @00436e10 InputEventTarget slot 1 (this = soul + 0x80), ret 8
    IDirect3DRMFrame *BuildBridge(Float3 from, Float3 to, Float3 color);   // @00436fc0 ret 0x24: a coloured strip between two spots
    void  UpdateRegion();                                   // @004373d0 slot 5 (named UpdateRegion: SqueezilSoul::Update is another function)
    int   LinkOpen(PlaceDescriptor *here, PlaceDescriptor *to);   // @00437e40 ret 8: the player may go from here to there
    int   BeenTo(int place);                                // @00437f40 ret 4: the pair (this region, place) is in "PlacesBeenTo"
    void  BlinkSelection(int selected);                     // @00438020 ret 4
    void  ShowBridge(int target);                           // @00438110 ret 4: the strip towards the aimed spot (none: takes it away)
    // slot 1 @00438310 GetInterface: always NULL; slots 6, 7, 39 and 40 do nothing
};
static_assert(sizeof(SqueezilRegionSoul) == 0xe8, "SqueezilRegionSoul layout");
