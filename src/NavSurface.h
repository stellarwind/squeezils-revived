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
    // @0040bee0 deletes the nav mesh, then the type tables and the two names
    void Destruct();
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
    float *ToWorldBefore(float *out, const float *point);    // @004128a0 like ToWorld with the matrix before the last Move
    float *MatrixToLocal(float *out, const float *m);        // @004110b0 a world matrix written in the surface's space
    float *Carried(float *out, const float *point);          // @0040d370 how far a point that stood still on the surface was carried by the last Move
    // The destructor the original inlines where it frees an instance: the flags, then the property table
    void Destruct();
};
static_assert(sizeof(InstancedSurface) == 0xf4, "InstancedSurface layout");

// What a surface type means (0x1c bytes; a value of the place's type table, read from a [<type name>] section of the ini by
// ReadSurfaceType @0040dd30). The section's Navigable key picks the behaviour: "false" (not walkable), "true" (the default),
// "bounce", "slippery" or "flyThrough"; any other text counts as "true".
struct SurfaceTypeDescription {
    int    walkable;                    // 0x00 0 for Navigable=false, else 1 (copied into the instance's flags array for every type bound to this one; 1 for unknown types)
    int    bounce;                      // 0x04 Navigable=bounce: a squeezil landing on it bounces
    int    slippery;                    // 0x08 Navigable=slippery
    int    flyThrough;                  // 0x0c Navigable=flyThrough: a flight passes through it and runs the activate script
    float  rebudGoodies;                // 0x10 the section's RebudGoodies key (default 0.1; what it is used for is not known)
    String activate;                    // 0x14 the script function that runs on entering the type (Activate key)
    String deactivate;                  // 0x18 the script function that runs on leaving it (Deactivate key)
};
static_assert(sizeof(SurfaceTypeDescription) == 0x1c, "SurfaceTypeDescription layout");

typedef Table<String, SurfaceTypeDescription *, Array<SurfaceTypeDescription *>, 0> SurfaceTypeTable;
typedef Set<String, 11> NameSet;             // a sorted set of surface type names (RTTI Set<SqzString,'L'>)

// Container vtables of the list's members (RTTI addresses)
template<> struct OriginalVtables<PrimitiveArray<String, 11> >             { enum : unsigned { primary = 0x4a00dc, bufferObject = 0x4a00c4 }; };
template<> struct OriginalVtables<NameSet>                                 { enum : unsigned { primary = 0x4a0114, bufferObject = 0x4a00fc }; };
template<> struct OriginalVtables<Array<NameSet *> >                       { enum : unsigned { primary = 0x4a0c44 }; };
template<> struct OriginalVtables<Array<LoadedSurface *> >                 { enum : unsigned { primary = 0x4a0c64 }; };
template<> struct OriginalVtables<Array<InstancedSurface *> >              { enum : unsigned { primary = 0x4a0c84 }; };
template<> struct OriginalVtables<Array<SurfaceTypeDescription *> >        { enum : unsigned { primary = 0x4a0ca4 }; };
template<> struct OriginalVtables<SurfaceTypeTable>                        { enum : unsigned { primary = 0x4a0cc4 }; };

struct SurfaceInfo;      // SqueezilSoul.h: what a squeezil stands on (type number, type name, what it is bound to)

// The part of the place's surface list the functions here touch (the object is PlaceSimulation + 0x3c).
struct NavSurfaceList {
    char                      path[0x100];    // 0x00 scratch: the .nat file name the ground loader builds (LoadGround)
    NameSet                   knownNames;      // 0x100 every surface type name defined so far ("none" first); a name may be defined once
    Array<NameSet *>          fileNames;       // 0x114 per ini file read (ReadSurfaceList): the names it defined, so the file can be unloaded again
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
    int   Bounces(const Contact *c);                                                            // @0040f610 the face's type is Navigable=bounce (the reconstruction first called it BlocksWalking)
    int   IsSlippery(const Contact *c);                                                         // @0040f770
    int   FlyThrough(const Contact *c);                                                         // @0040f8d0 Navigable=flyThrough
    String *ActivateScript(String *out, const Contact *c);                                      // @0040f9b0 the type's Activate script; empty when unknown
    float RebudGoodies(const Contact *c);                                                       // @0040fb20 0 when unknown
    int   IsGone(const Contact *c);                                                             // @00410890 the instance is queued for removal
    void  Remove(int instance);                                                                 // @004102f0 queues it
    void  MoveInstance(int instance, const float *matrix);                                      // @00410310
    int   Load(IniReader *ini, const char *navFile, const char *natFile);                       // @0040fc10 the loaded surface's number (0 when the .nav file failed to load)
    int   AddInstance(NavProp *prop, int surface, const float *matrix, IniValueTable *properties);   // @004100b0 the new instance's number
    int   LoadGround(IniReader *ini);                                                            // @0040e480 the place's own ground (see NavSurface.cpp)

    // ---- reading the ini ----
    int   ReadSurfaceList(IniReader *ini);                                  // @0040d870 [InputFiles] and [SurfaceList]; 1 (a missing list is only logged)
    int   ReadSurfaceNames(IniReader *ini, NameSet *names);                 // @0040dad0 the [SurfaceList] section of one ini file; 0 when it is empty
    int   ReadSurfaceType(IniReader *ini, const char *name);                // @0040dd30 the [name] section: a SurfaceTypeDescription
    void  UnloadLastFile();                                                 // @0040e160 forgets the names (and descriptions) of the file read last
    void  SetFaceSet(String faceSet, String type, const Contact *c);        // @0040f450 binds a face set of the contact's surface copy to a type (the Strings are taken over)
    void  TypeChanged(void *soul, const Contact *c, const SurfaceInfo *oldInfo, const SurfaceInfo *newInfo);   // @00412920 runs the old type's Deactivate and the new one's Activate script

    // ---- lifetime ----
    NavSurfaceList *Construct();                                            // @0040d530
    void  Destruct();                                                       // @0040d640 destroys every loaded surface and instance
    void  DestroyInstance(int number);                                      // the inlined instance destructor + free (see @004106e0)
    void  FlushRemoved();                                                   // @004106e0 once a frame: really removes what was queued a frame ago, then moves the new queue to "later"
    void  MoveCarried();                                                    // @00410330 once a frame: bodies standing on a surface that moved are carried along with it

    // ---- moving over the surfaces (the nav mesh does the geometry; see NavMeshTrace / NavMeshMove in Original.h) ----
    // @004108e0 Whether a flight of `distance` from pos along direction hits a surface copy; if so the nearest hit: where, how the
    // squeezil stands there (right/up/forward in the world) and the contact (copy, face, and the matrix in the copy's space).
    int   FlightHitsSurface(const Float3 *pos, const Float3 *right, const Float3 *up, const Float3 *forward, const Float3 *direction,
                            float distance, Float3 *hitPos, Float3 *newRight, Float3 *newUp, Float3 *newForward, Contact *newContact);
    // @00411120 Moves a squeezil `distance` along the surface it stands on (contact), from copy to copy when it runs off an edge onto
    // another one nearby; writes the new place, the way it now goes and the new contact. 0 when it can go nowhere. The result of the
    // nav mesh's own move is returned as it is. Runs a surface type's Activate script when it flies through such a surface.
    int   MoveAlongSurface(void *soul, const Float3 *pos, const Float3 *right, const Float3 *up, const Float3 *forward, const Float3 *direction,
                           float distance, const Contact *contact, Float3 *newPos, Float3 *newRight, Float3 *newUp, Float3 *newForward,
                           Float3 *newDirection, Contact *newContact);
};

// Reads the [SurfaceBindingList] section: type name -> the type it is bound to. @0040e2e0
void ReadSurfaceBindings(IniReader *ini, IniValueTable *out);
static_assert(offsetof(NavSurfaceList, knownNames) == 0x100 && offsetof(NavSurfaceList, fileNames) == 0x114, "NavSurfaceList head layout");
static_assert(offsetof(NavSurfaceList, loaded) == 0x124 && offsetof(NavSurfaceList, types) == 0x16c, "NavSurfaceList layout");

// The conversions between a surface's space and the world (cdecl in the original). `identity` non-zero means the matrix is the
// identity and the input is copied. `inverse`, when given, is the matrix's turn already transposed (translation 0): the caller
// has it at hand; otherwise it is made here.
float *PointToLocal(float *out, const float *point, const float *m, int identity, const float *inverse);   // @0040c8a0
float *MatrixToLocal(float *out, const float *a, const float *m, int identity);                            // @0040cb30
float *PointToWorld(float *out, const float *point, const float *m, int identity, const float *turn);      // @0040ce30
float *MatrixToWorld(float *out, const float *a, const float *m, int identity);                            // @0040d070

// @00412830 (also @00409f00, the same code) the matrix with right, up, forward and the position as its four rows (the fourth column 0 0 0 1)
float *MakeMatrix(float *out, const float *right, const float *up, const float *forward, const float *pos);

// @0040eec0 (stdcall) Reads a .nat file: "#Nat V1.0 ascii", the number of faces, then one "face typeName" line per face. Writes each face's
// type number into the mesh. Numbers follow `types`: it must start with " none" (number 0); a name not in it yet is appended. Returns the
// number of faces in the file, 0 on any error (also logged). `directory` is not used. A file that cannot be opened gives a Retry/Cancel box.
int ReadSurfaceTypes(void *mesh, const char *directory, const char *natFile, StringList *types);
