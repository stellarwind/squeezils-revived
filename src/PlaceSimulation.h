// PlaceSimulation: the inside of one place (a level the player enters from a region's tree). Reconstructed from SQZ.EXE
// 0x438850-0x43a000 (RTTI class PlaceSimulation, vtable 0x49e224). The simulation in the global at 0x4badbc is one of these while
// the player is in a place, and a RegionSimulation (RegionSimulation.h) while they walk about the region.
//
// A place lives in the folder of its region: <region folder>\place<NN>\place.ini (NN is the place number, two digits) unless the
// global string at 0x4c7348 names another file. The place's ground is a set of walkable surfaces (NavSurface.h, the object at
// +0x3c), its contents (creatures, goodies, where the player starts) come from the place's level file (the object at +0x38, a
// class of the game's that is not reconstructed yet), and its creatures live in the LocationSimulation's table.
//
// Slot list: 0 Destroy, 1 Init (loads the place), 2 Clear, 3 UpdateSouls (frozen when `finished`), 4 RunActors, 7 ClearCreatures,
// 8 CollideCreatures (the surfaces carry what stands on them first), 9 CanSave (0), 10 Load, 11 Save, 12 Velocity (of the
// surface under a body).
#pragma once
#include "Simulation.h"
#include "SqueezilBody.h"
#include "Float3.h"

struct NavSurfaceList;
class IniReader;
struct Possessions;

// A creature the level file puts in the place (0xb0 bytes, the same shape as a BodyPlacement with a name where the matrix is)
struct LevelCreature {
    int     kind;             // 0x00
    char    name[64];         // 0x04 the creature's template (the name Find asks the level file for)
    Contact contact;          // 0x44
    Float3  position;         // 0x8c
    Float3  up;               // 0x98
    Float3  direction;        // 0xa4
};
static_assert(sizeof(LevelCreature) == 0xb0, "LevelCreature layout");

// The place's level file object (0x2f4 bytes, made by @0044af30; its class has no RTTI name and is not reconstructed). Only the
// parts the simulation touches are modelled.
struct PlaceLevel {
    int            loaded;              // 0x000 nonzero when the file gave a start for the player (@0044bd60 read the level file)
    char           unknown004[0x1f4];
    BodyPlacement  start;               // 0x1f8 where the player starts (the file's, or made from the default contact when there is none)
    Array<LevelCreature *> creatures;   // 0x2a8 the creatures the file puts in the place, in file order
    Array<int>     ids;                 // 0x2b8 the creature table ids the simulation gave them (the player's 0 first)
    char           unknown2c8[0x2c];
};
static_assert(offsetof(PlaceLevel, start) == 0x1f8, "PlaceLevel layout");

struct PlaceSimulation : LocationSimulation {
    int             region;         // 0x24 the region the place is in
    int             place;          // 0x28 the place's number (names the folder)
    int             location;       // 0x2c the spot of the region the player came from (saved, so the game can go back there)
    int             levelType;      // 0x30 [Type] LevelType of place.ini
    int             finished;       // 0x34 set by Finish: the souls stop deciding, only the player's soul is told
    PlaceLevel     *level;          // 0x38
    NavSurfaceList *surfaces;       // 0x3c the place's walkable surfaces
    void           *playerSoul;     // 0x40 the SqueezilPlayerSoul

    PlaceSimulation *Construct(int region, int place, int location);   // @00438850 ret 0xc
    void  Destruct();                                        // @004388f0
    PlaceSimulation *DestroyAndFree(int flags);              // @004388d0 slot 0
    void  Init();                                            // @00438940 slot 1: the place's ini files, the scene, the surfaces, the music
    void  Start();                                           // @00438ee0 the player, the creatures and the camera
    void  Clear();                                           // @00439a50 slot 2
    void  UpdateSouls();                                     // @00439b00 slot 3
    void  ClearCreatures();                                  // @00439a00 slot 7
    void  CollideCreatures();                                // @00439ae0 slot 8
    int   CanSave() { return 0; }                            // @00439f80 slot 9
    int   Load(const char *path);                            // @00439b70 slot 10 (ret 4): see RegionSimulation::Load (replaces this simulation)
    int   Save(const char *path);                            // @00439db0 slot 11 (ret 4)
    Float3 *Velocity(Float3 *out, void *body);               // @00439e90 slot 12 (ret 8): how fast the surface under the body is moving, 0 for a body in the air
    void  Finish() { finished = 1; }                         // @00439ad0
    PlaceLevel *GetLevel() { return level; }                 // @00439b50
    NavSurfaceList *GetSurfaces() { return surfaces; }       // @00439b60
    BodyPlacement *StartPlacement() { return &level->start; }   // @00439f70 (on the level object)
};
static_assert(sizeof(PlaceSimulation) == 0x44, "PlaceSimulation layout");
