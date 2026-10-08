// NavSurface: the walkable geometry of a place (nothing to do with DirectDraw surfaces). Reconstructed from SQZ.EXE 0x40bc60-0x412c40.
// The three classes keep the names the RTTI of their container instantiations gives (Array<LoadedSurface*>, Array<InstancedSurface*>,
// Array<SurfaceTypeDescription*>); NavSurfaceList, NavMeshFaces and IdList are our names (no RTTI).
//
// A level's ground is made of surfaces. A *loaded surface* is a .nat file read once (its faces, and the surface types the faces
// refer to by number). An *instanced surface* is a copy of a loaded surface standing in the world with its own matrix: the level's
// own ground is one, and every NavProp (a raft, a lily pad) adds one that moves with the prop. A creature standing on a surface
// is described by a Contact: `face` is the number of the instance, `second` the face within it.
//
// Every face has a surface type number; the loaded surface keeps the names of its types in order. An instance keeps a table
// "type name -> what that type is called here", all starting as "none": a level can bind a type of the surface to another
// type (SurfaceBinding in a prop's definition), and that is what the squeezil's script sees. The place keeps a table of
// SurfaceTypeDescription (what a type name means for walking: blocks walking, slippery, runs a script when touched).
//
// The list that holds all of this belongs to the PlaceSimulation (GetPlaceSurfaces). Only the members the functions here use
// are modelled; the rest of the object is the original's.
//
// Matrices are 16 floats, row vectors: rows 0-2 turn, row 3 is the position. An instance remembers the matrix it had one move
// ago, so that the game can tell how far a point standing on the surface was carried.
#pragma once
#include <stddef.h>
#include "Container.h"
#include "Body.h"
#include "VirtualCall.h"

class String;

// The .nat file object (0x2030 bytes, original class; World.h holds one too). Only the face array is read here.
struct NavMeshFaces {
    char  pad0[0x0c];
    char *faces;                // 0x0c faces, 0x28 bytes each; +0x24 of a face is its surface type number
};
inline int NavMeshFaceType(const void *mesh, int face)
{
    return *(const int *)(((const NavMeshFaces *)mesh)->faces + face * 0x28 + 0x24);
}

typedef BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21> SurfaceTypeNumbers;

// The game's sorted list of numbers (a Set<int> with a BufferObject on the end, vtable @004a009c; the same class as the id list in
// Simulation.h). Its virtual functions are called through the original table: slot 1 inserts (by reference), slot 6 asks.
struct IdList {
    Array<int> ids;               // 0x00
    void      *bufferObject;      // 0x10
    int  Contains(int id) { return CallVirtual<int>(this, 6, &id); }
    void Insert(int id)   { CallVirtual<int>(this, 1, &id, -1); }
    int  At(int index)    { return ids.data[index]; }
    void RemoveAt(int index) { CallVirtual<void>(this, 2, index); }
};
static_assert(sizeof(IdList) == 0x14, "IdList layout");

// 0x58 bytes. Built by @0040bc60.
struct LoadedSurface {
    void              *mesh;            // 0x00 the .nav object (0x2030 bytes, World.h holds one too)
    float              sizeSquared;     // 0x04 the mesh's float at +0x2020 (@0047af20); the instance tests squared distances against it
    int                faceCount;       // 0x08 the number of faces in the .nat file (what ReadSurfaceTypes returned)
    String             directory;       // 0x0c the ini file's folder the surface was loaded from
    String             file;            // 0x10 the .nat file (LoadSurface reuses a loaded surface with the same folder and .nat file)
    StringList         typeNames;       // 0x14 the surface types, in the order the faces number them (the first is " none")
    SurfaceTypeNumbers typeNumbers;     // 0x28 type name -> number

    // @0040bc60 types (copied) are the .nat file's type names; the size is read from the mesh
    LoadedSurface *Init(void *navMesh, int faces, const char *directory, const char *natFile, StringList *types);
};
static_assert(sizeof(LoadedSurface) == 0x58, "LoadedSurface layout");

// 0xf4 bytes. Built by @0040bf90; destroyed by @0040bee0.
struct InstancedSurface {
    LoadedSurface *surface;             // 0x00
    float          matrix[16];          // 0x04 where the surface stands now
    int            identity;            // 0x44 matrix is the identity (then every conversion is a copy)
    float          previous[16];        // 0x48 where it stood before the last Move
    int            previousIdentity;    // 0x88
    int            moveDirty;           // 0x8c set by Move; Changed() recomputes its answer when set
    int            changed;             // 0x90 the answer: 0 same as before the move, else the sign of the first differing byte
    struct NavProp *prop;               // 0x94 the prop this copy belongs to (NULL for the level's own ground)
    IniValueTable  properties;          // 0x98 surface type name -> the type it stands for here
    IntList        flags;               // 0xc8 per surface type: 1, or the flag of the description its binding names (AddInstance sets it)
    float          position[3];         // 0xdc the matrix's position, (0,0,0) when the matrix is the identity
    float          previousPosition[3]; // 0xe8

    // @0040bf90 loaded must be the surface's entry in the list; properties are laid over the "none" defaults
    InstancedSurface *Init(NavProp *prop, LoadedSurface *loaded, const float *matrix, IniValueTable *properties);
    void  Move(const float *newMatrix);                      // @0040c470 slides the current matrix into `previous`, takes the new one
    bool  IsNear(const float *point);                        // @0040c700 within the surface's size of its position now
    bool  WasNear(const float *point);                       // @0040c7b0 the same for the position before the last Move
    int   Changed();                                         // @0040c860 whether the matrix differs from the previous one (cached)
    float *ToLocal(float *out, const float *point);          // @0040f6f0 a world point in the surface's own space
    float *ToWorld(float *out, const float *point);          // @0040f850 a point of the surface in the world
    float *MatrixToWorld(float *out, const float *m);        // @004105f0 (returns the 16 floats in out)
    float *ToLocalBefore(float *out, const float *point);    // @00410660 like ToLocal with the matrix before the last Move
    float *Carried(float *out, const float *point);          // @0040d370 how far a point that stood still on the surface was carried by the last Move
};
static_assert(sizeof(InstancedSurface) == 0xf4, "InstancedSurface layout");

// What a surface type means for walking (a value of the place's type table; built elsewhere).
struct SurfaceTypeDescription {
    int    flag;                        // 0x00 copied into the instance's flags array for every type that is bound to this one (1 for unknown types)
    int    blocksWalking;               // 0x04
    int    slippery;                    // 0x08
    int    hasTouchScript;              // 0x0c
    float  value;                       // 0x10 (a number; what it is used for is not known)
    String touchScript;                 // 0x14 the script function that runs when a squeezil lands on it
};

typedef Table<String, SurfaceTypeDescription *, Array<SurfaceTypeDescription *>, 0> SurfaceTypeTable;

// The part of the place's surface list the functions here touch (the object is PlaceSimulation + 0x3c).
struct NavSurfaceList {
    char                      unknown0[0x124];
    Array<LoadedSurface *>    loaded;          // 0x124
    Array<InstancedSurface *> instances;       // 0x134 (a removed instance leaves a NULL until the number is reused)
    IdList                    removed;         // 0x144 instances queued for removal (RemoveSurfaceInstance)
    IdList                    removedLater;    // 0x158 (second queue: only tested by IsGone here)
    SurfaceTypeTable          types;           // 0x16c

    LoadedSurface    *Loaded(int number)         { return loaded.At(number); }                  // @00412c10
    InstancedSurface *Instance(int number)       { return instances.At(number); }              // @00412bf0
    void *FaceMesh(const Contact *c);                                                           // @00412bc0 the .nat object of the contact's surface
    int   FaceType(const Contact *c);                                                           // @0040f380 the surface type number of the face
    IniValueTable *Properties(const Contact *c) { return &Instance(c->face)->properties; }     // @0040f3a0
    String *TypeName(String *out, const Contact *c);                                            // @0040f3c0 the face's type, by name
    SurfaceTypeDescription *Description(const Contact *c);                                      // inlined in the five below: NULL when unknown
    int   BlocksWalking(const Contact *c);                                                      // @0040f610
    int   IsSlippery(const Contact *c);                                                         // @0040f770
    int   HasTouchScript(const Contact *c);                                                     // @0040f8d0
    String *TouchScript(String *out, const Contact *c);                                         // @0040f9b0 empty when unknown
    float TypeValue(const Contact *c);                                                          // @0040fb20 0 when unknown
    int   IsGone(const Contact *c);                                                             // @00410890 the instance is queued for removal
    void  Remove(int instance);                                                                 // @004102f0 queues it
    void  MoveInstance(int instance, const float *matrix);                                      // @00410310
    int   Load(IniReader *ini, const char *navFile, const char *natFile);                       // @0040fc10 the loaded surface's number (0 when the .nav file failed to load)
    int   AddInstance(NavProp *prop, int surface, const float *matrix, IniValueTable *properties);   // @004100b0 the new instance's number
};
static_assert(offsetof(NavSurfaceList, loaded) == 0x124 && offsetof(NavSurfaceList, types) == 0x16c, "NavSurfaceList layout");

// The conversions between a surface's space and the world (cdecl in the original). `identity` non-zero means the matrix is the
// identity and the input is copied. `inverse`, when given, is the matrix's turn already transposed (translation 0): the caller
// has it at hand; otherwise it is made here.
float *PointToLocal(float *out, const float *point, const float *m, int identity, const float *inverse);   // @0040c8a0
float *MatrixToLocal(float *out, const float *a, const float *m, int identity);                            // @0040cb30
float *PointToWorld(float *out, const float *point, const float *m, int identity, const float *turn);      // @0040ce30
float *MatrixToWorld(float *out, const float *a, const float *m, int identity);                            // @0040d070

// @0040eec0 (stdcall) Reads a .nat file: "#Nat V1.0 ascii", the number of faces, then one "face typeName" line per face. Writes each face's
// type number into the mesh. Numbers follow `types`: it must start with " none" (number 0); a name not in it yet is appended. Returns the
// number of faces in the file, 0 on any error (also logged). `directory` is not used. A file that cannot be opened gives a Retry/Cancel box.
int ReadSurfaceTypes(void *mesh, const char *directory, const char *natFile, StringList *types);
