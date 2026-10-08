// Prop: things placed in a level that have a D3DRM frame: meshes, sprites, and (not yet reconstructed) frame groups, level of detail
// switches, cycling props and nav props. Reconstructed from SQZ.EXE 0x425e50-0x426c70. The classes are the original's (RTTI):
//
//     Prop (base, 0x40 bytes, vtable 0x49db4c)
//       MeshProp   (0x44 bytes, vtable 0x49db8c)   shows a loaded geometry (Resources.h: FindGeometry) under a frame
//       SpriteProp (0x70 bytes, vtable 0x49dbe4)   shows a decal (a texture from FindTexture) under a frame
//       FrameProp -> LODProp, CycleProp -> NavProp  still original code
//
// A prop loaded from the level's PropList is a template; Clone makes the instance the game places (owner set, isInstance 1) and
// Instantiate then gives that instance its own position, scale, direction and up vector. Show builds the frame under a parent
// frame, Hide takes it out again.
//
// The objects are shared with original code that calls their virtual functions through the vtable at +0, so there is no C++
// virtual here: the object carries the original vtable pointer, the V... members dispatch through it, and the plain members are
// the implementations (each one is hooked at its original address). Slots with a trivial body (no-ops, constant results) stay
// original and are not listed with an implementation.
//
// Prop vtable slots:  0 Destroy(flags)  1 Clone(owner)  2 Instantiate  3 BuildFrame  4 Show  5 Hide  6 ApplyScale  7 no-op(int)
//                     8 no-op  9 GetOwner  10 GetAttachFrame  11 returns 0 (1 arg)  12 returns 0  13 returns 0 (a String by value)  14 IsShown
#pragma once
#include "Float3.h"
#include "VirtualCall.h"
#include "GameString.h"
#include "World.h"
#include "Original.h"

// The slot numbers of Prop's virtual functions (the order of the original vtable, shared by every prop class)
enum PropSlot {
    PROP_DESTROY = 0, PROP_CLONE = 1, PROP_INSTANTIATE = 2, PROP_BUILD_FRAME = 3, PROP_SHOW = 4, PROP_HIDE = 5,
    PROP_APPLY_SCALE = 6, PROP_GET_OWNER = 9, PROP_GET_ATTACH_FRAME = 10, PROP_IS_SHOWN = 14
};

struct Prop {
    void  **vtbl;                   // 0x00
    Float3  position;               // 0x04
    Float3  scale;                  // 0x10
    Float3  direction;              // 0x1c forward
    Float3  up;                     // 0x28
    IDirect3DRMFrame *frame;        // 0x34 NULL while the prop is hidden
    void   *owner;                  // 0x38 what made this instance (set by Clone's argument); type not identified
    int     isInstance;             // 0x3c 1 for a prop made by Clone

    Prop *Init(Float3 position, Float3 scale, Float3 direction, Float3 up);   // @00425e50 constructor: the four vectors, no frame, no owner
    void  Destruct();                                                         // @00426080 destructor: hides the prop
    Prop *Destroy(int flags);                                                 // @00425f80 slot 0 (bit 0 of flags: free the memory)
    Prop *Instantiate(void *owner, Float3 position, Float3 scale, Float3 direction, Float3 up);   // @00425fa0 slot 2: Clone, then set the four vectors
    IDirect3DRMFrame *Show(IDirect3DRMFrame *parent);                         // @00426120 slot 4: BuildFrame if hidden (logs a warning if not)
    int   IsShown();                                                          // @00426160 slot 14
    void  Hide();                                                             // @00426170 slot 5: takes the frame out of its parent
    IDirect3DRMFrame *GetAttachFrame();                                       // @00426230 slot 10: the owner's part frame, else the frame's parent

    // The virtual calls: each looks the function up in the object's own vtable (see VirtualCall.h), so the version of the
    // object's class runs. The number is the slot in the list above.
    Prop *VClone(void *owner);                                                // slot 1
    IDirect3DRMFrame *VBuildFrame(IDirect3DRMFrame *parent);                  // slot 3
    void  VApplyScale(IDirect3DRMFrame *frame, Float3 scale);                 // slot 6
};
static_assert(sizeof(Prop) == 0x40, "Prop layout");

struct MeshProp : Prop {
    String name;                    // 0x40 the geometry's name in the geometry tables

    MeshProp *Init(const char *name, Float3 position, Float3 scale, Float3 direction, Float3 up);   // @004262e0
    void  Destruct();                                                         // @00426580
    Prop *Destroy(int flags);                                                 // @00426450 slot 0
    Prop *Clone(void *owner);                                                 // @00426470 slot 1
    IDirect3DRMFrame *BuildFrame(IDirect3DRMFrame *parent);                   // @004265f0 slot 3: the geometry under a new frame, placed and scaled
};
static_assert(sizeof(MeshProp) == 0x44, "MeshProp layout");

struct SpriteProp : Prop {
    String        name;             // 0x40 the texture's name in the texture tables
    DecalSettings settings;         // 0x44

    SpriteProp *Init(const char *name, Float3 position, Float3 scale, const DecalSettings *settings);   // @004267b0
    void  Destruct();                                                         // @00426a70
    Prop *Destroy(int flags);                                                 // @00426980 slot 0
    Prop *Clone(void *owner);                                                 // @004269a0 slot 1
    IDirect3DRMFrame *BuildFrame(IDirect3DRMFrame *parent);                   // @00426b00 slot 3: a decal of the texture; NULL (and an error in the log) when the texture is missing
    void  ApplyScale(IDirect3DRMFrame *frame, Float3 scale);                  // @00426c00 slot 6: the decal's size is the scale's x and y
};
static_assert(sizeof(SpriteProp) == 0x70, "SpriteProp layout");
