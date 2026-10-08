// NavProp: a CycleProp that is also a walkable surface. Its constructor loads a surface (a .nat file, from the directory of the
// ini file the prop comes from) into the current place, and while the prop is shown a copy of that surface sits where the prop
// is: BuildFrame adds it, Update moves it with the prop every frame, Hide takes it away. Creatures can walk on it, so a moving
// NavProp (a raft, a lily pad) carries them. Reconstructed from SQZ.EXE 0x429380-0x429dc0 (class name and vtable 0x49ddac from
// RTTI).
//
// The surfaces belong to the PlaceSimulation, which is not reconstructed: the calls into it are in Original.h.
//
// A NavProp also carries a table of named properties for its surface copy: every property name the surface declares, set to
// "none", then the values the prop's own definition gives. The table goes to the place with the copy.
//
// Note: NavProp::Update runs FrameProp's update, not CycleProp's, so a NavProp never steps through its children.
//
// NavProp's own vtable slots: 0 Destroy  1 Clone  3 BuildFrame  5 Hide  7 Update(seconds)  11 GetSizeSquared(&out)
//                             25 GetSurface  26 GetInstance
#pragma once
#include "CycleProp.h"

class IniReader;

// A loaded walkable surface (the place's class). Only the fields NavProp reads are known.
struct NavSurface {
    int           unknown0;         // 0x00
    float         sizeSquared;      // 0x04 copied into a NavProp (slot 11; LODProp reads it as a size squared)
    int           unknown8[3];      // 0x08
    Array<String> propertyNames;    // 0x14 the properties a copy of the surface has
};

enum NavPropSlot { NAVPROP_GET_SIZE_SQUARED = 11, NAVPROP_GET_SURFACE = 25, NAVPROP_GET_INSTANCE = 26 };

struct NavProp : CycleProp {
    int           surface;          // 0xf4 the place's number for the loaded surface
    int           instance;         // 0xf8 the place's number for the copy shown with this prop (set by BuildFrame)
    IniValueTable properties;       // 0xfc property name -> value, handed to the place with the copy (0x30 bytes)
    float         sizeSquared;      // 0x12c from the surface (not set by the copy constructor; Clone copies it)

    // @00429380 the one the prop loader uses: loads the surface named surfaceName from natFile (the directory is ini's), then
    // the property table (the surface's names set to "none", then the given values)
    NavProp *Init(Float3 position, Float3 scale, Float3 direction, Float3 up, StringList *childNames, MorphCycle *cycle,
                  CycleMessageTable *messages, StepEventTable *events, float period, int carry, float startTime,
                  IniReader *ini, const char *surfaceName, const char *natFile, IniValueTable *values);
    // @004296d0 the one Clone uses: an already loaded surface and a whole property table
    NavProp *InitCopy(Float3 position, Float3 scale, Float3 direction, Float3 up, StringList *childNames, MorphCycle *cycle,
                      CycleMessageTable *messages, StepEventTable *events, float period, int carry, float startTime,
                      int surface, IniValueTable *properties);
    void  Destruct();                                                         // @00429a40
    Prop *Destroy(int flags);                                                 // @004296b0 slot 0
    Prop *Clone(FrameProp *owner);                                            // @004298f0 slot 1
    IDirect3DRMFrame *BuildFrame(IDirect3DRMFrame *parent);                   // @00429ab0 slot 3: CycleProp's, then adds the surface copy
    void  Hide();                                                             // @00429c20 slot 5: FrameProp's, then removes the copy
    void  Update(float seconds);                                              // @00429c60 slot 7: FrameProp's, then moves the copy
    int   GetSizeSquared(float *out);                                         // @00429da0 slot 11: always 1 (the only prop with a size)
    int   GetSurface() { return surface; }                                    // @00429690 slot 25
    int   GetInstance() { return instance; }                                  // @004296a0 slot 26
};
static_assert(sizeof(NavProp) == 0x130, "NavProp layout");
