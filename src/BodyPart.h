// BodyPart and AnimatedBody: a character built of parts (foot, hand, body ...), and the player that moves them.
// Reconstructed from SQZ.EXE 0x41d870-0x41e30d.
//
// A BodyPart owns a frame in the scene and, when it has a model, a morphing mesh built from it. Every frame the AnimatedBody
// hands each part its PartPose (Animation.h): the part's placement plus the morph weights. An AnimatedBody holds the parts by
// name, the clip (MorphCycle) it is playing with the time in it, and the pose table the clip is sampled into.
//
// Both classes have virtual methods in the original (RTTI "BodyPart" and "AnimatedBody"); the objects are made by the game's
// own code, so here the vtable pointer is kept as a plain first field and the methods are non-virtual. The slot numbers are
// listed next to each method.
#pragma once
#include "SqgModel.h"
#include "Animation.h"

class Scene;

struct BodyPart {
    void             *vtbl;       // 0x00 vtable 0x49d7c4: slot 0 Apply, slot 1 Build, slot 2 returns the frame
    SqgModel         *model;      // 0x04 NULL for a part without geometry of its own
    PZMorphMesh      *morph;      // 0x08 the mesh and the working vertices, created by Build when there is a model
    IDirect3DRMFrame *frame;      // 0x0c

    BodyPart *Init(SqgModel *model);                           // @0041d870 no frame or mesh yet
    BodyPart *Init();                                          // @0041d8a0 no model either
    BodyPart *Init(const BodyPart *like);                      // @0041d8c0 the same model, nothing built
    void Destruct();                                           // @0041d8e0 frees the mesh
    void Apply(IDirect3DRMFrame *ref, const PartPose *pose);   // @0041d900 slot 0 (thiscall, 2 stack args)
    void Build(IDirect3DRMFrame *parent);                      // @0041d990 slot 1 (thiscall, 1 stack arg)
};

// Where the player is in its clip when it is not blending: loopMode 1 starts again at the beginning, -1 plays back and forth,
// 0 stops at the end. direction is 1 (forward), -1 (backward) or 0 (stopped).
struct AnimatedBody {
    void             *vtbl;           // 0x00 vtable 0x49d7d4
    BodyPartTable     partTable;      // 0x04 the parts by name (0x28 bytes)
    MorphCycle       *cycle;          // 0x2c the clip being played
    float             time;           // 0x30 seconds into the clip
    int               hint;           // 0x34 the sample interval the last sampling ended in (a start for the next search)
    int               direction;      // 0x38
    int               loopMode;       // 0x3c
    PoseTable        *pose;           // 0x40 the pose right now (made when the first clip is set)
    PoseTable        *previousPose;   // 0x44 while changing clip: the pose the old clip ended in; the new clip is blended in
    float             blendLeft;      // 0x48 seconds of the blend that remain
    IDirect3DRMFrame *rootFrame;      // 0x4c parent of every BodyPart's frame

    AnimatedBody *Init();                                           // @0041d9e0
    AnimatedBody *InitCopy(const AnimatedBody *like);               // @0041dab0 copies of the parts, no frames, no clip
    void Destruct();                                                // @0041dc70 deletes the parts and the poses
    int  AddPart(const char *name, BodyPart *part);                 // @0041dd30 slot 0
    void Build(IDirect3DRMFrame *parent);                           // @0041dda0 slot 1: makes the root frame under parent and builds every part under it
    void DestroyFrames(IDirect3DRMFrame *parent);                   // @0041ddf0 slot 2: removes the root frame from parent and drops it
    void SetCycle(MorphCycle *clip, int blend);                     // @0041de20 slot 3: start a clip; with blend the old pose is blended into it over its period
    void ApplyPose();                                               // @0041df10 slot 4: hands every part its pose
    void Advance(float seconds, int skipApply);                     // @0041df50 slot 5: moves the time on (per direction and loop mode), samples the clip, applies the pose
    void SetDirection(int d);                                       // @0041e2d0 slot 6 (only the sign counts)
    void SetLoopMode(int m);                                        // @0041e2f0 slot 7 (only the sign counts)
    float GetTime() { return time; }                                // @0041da70 slot 8
    void SetRootTransform(IDirect3DRMFrame *ref, const float *m);   // @0041e230 slot 9
    float GetBlendFraction();                                       // @0041e2a0 slot 10: how far the blend into the new clip is (0 .. 1), -1 when not blending
    float GetDuration() { return cycle->Duration(); }               // @0041da80 slot 11
    int   GetLoopMode() { return loopMode; }                        // @0041da90 slot 12
    IDirect3DRMFrame *GetRootFrame() { return rootFrame; }          // @0041daa0 slot 13
    void SetTime(float t) { time = t; }                             // @0041e290 slot 14
};
static_assert(sizeof(AnimatedBody) == 0x50, "AnimatedBody size");
