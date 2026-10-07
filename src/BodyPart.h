// BodyPart: one piece of a character (foot, hand, body...). It owns a frame in the scene and, when it has a model, a
// morphing mesh built from it. Every frame the game hands it a pose: the part's transform plus the morph weights.
// Reconstructed from SQZ.EXE 0x41d900-0x41d9ce. The class has virtual methods in the original (RTTI "BodyPart", vtable at
// 0x49d7c4, Apply and Build are slots 0 and 1); here the vtable pointer is kept as a plain first field and the methods
// are non-virtual, since the objects are created by the game's own code.
#pragma once
#include "SqgModel.h"

class Scene;

// The array of morph weights inside a pose: a game array object (own vtable) with the element count at +4.
// Its slot 4 (offset 0x10) returns a pointer to the float data.
struct WeightArray {
    void **vtbl;                  // 0x00
    int    count;                 // 0x04
    float *Data();                // calls vtbl[4]
};

struct BodyPose {
    float position[3];            // 0x00
    float direction[3];           // 0x0c
    float up[3];                  // 0x18
    WeightArray weights;          // 0x24 (embedded array object, its count is at 0x28)
};

struct BodyPart {
    void             *vtbl;       // 0x00
    SqgModel         *model;      // 0x04 NULL for a part without geometry of its own
    PZMorphMesh      *morph;      // 0x08 the mesh and the working vertices, created by Build when there is a model
    IDirect3DRMFrame *frame;      // 0x0c

    void Apply(IDirect3DRMFrame *ref, const BodyPose *pose);   // 0x41d900 (thiscall, 2 stack args)
    void Build(IDirect3DRMFrame *parent);                      // 0x41d990 (thiscall, 1 stack arg)
};

// AnimatedBody: a whole animated character (its BodyParts, the animation player and the root frame). Only the field and
// the method that touch D3DRM are reconstructed so far; the animation timing and blending code is game logic.
struct AnimatedBody {
    void             *vtbl;           // 0x00
    char              pad04[8];     // 0x04..0x0b
    int               partCount;    // 0x0c number of BodyParts
    char              pad10[0x18];  // 0x10..0x27
    BodyPart        **parts;        // 0x28 the BodyParts (array data)
    char              pad2c[0x20];  // 0x2c..0x4b animation state, poses
    IDirect3DRMFrame *rootFrame;    // 0x4c parent of every BodyPart's frame

    void SetRootTransform(IDirect3DRMFrame *ref, const float *m);   // 0x41e230 (thiscall, 2 stack args)
    void Build(IDirect3DRMFrame *parent);                           // 0x41dda0 makes the root frame under parent and builds every part under it
    void DestroyFrames(IDirect3DRMFrame *parent);                   // 0x41ddf0 removes the root frame from parent and drops it
};
