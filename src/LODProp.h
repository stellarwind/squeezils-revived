// LODProp: a FrameProp whose children are the same thing at different levels of detail; only one child is shown at a time,
// chosen by how far the camera is from the prop. Reconstructed from SQZ.EXE 0x428050-0x4288b0 (class name and vtable 0x49dcb4
// from RTTI). Everything else (Hide, Update, FindChild, the cycle setters) is FrameProp's.
//
// "distances" has one number per child: the first child is shown while the squared distance to the camera is under the first
// number, the second under the second, and so on; the last child is shown from there on. (The numbers are compared with the
// squared distance, so the level file has to give them squared.) The constructor repeats the last number to give every child one.
// Levels are numbered from 0; -1 means nothing is shown (the prop is behind the camera and out of its own size, see SelectLevel).
//
// LODProp's own vtable slots: 0 Destroy  1 Clone  3 BuildFrame  8 UpdateLevel (a Prop no-op slot, here: re-choose the child)
#pragma once
#include "FrameProp.h"
#include "Animation.h"

enum LODPropSlot { LODPROP_UPDATE_LEVEL = 8 };

struct LODProp : FrameProp {
    int        level;               // 0xac the child on show (-1 none); starts at 0
    WeightList distances;           // 0xb0 squared distances: child i is on show while the camera is nearer than distances[i]

    LODProp *Init(Float3 position, Float3 scale, Float3 direction, Float3 up, StringList *childNames, MorphCycle *cycle,
                  CycleMessageTable *messages, WeightList *distances);        // @00428050 pads the distances up to the number of children
    void  Destruct();                                                         // @00428260
    Prop *Destroy(int flags);                                                 // @00428240 slot 0
    Prop *Clone(FrameProp *owner);                                            // @004282d0 slot 1
    IDirect3DRMFrame *BuildFrame(IDirect3DRMFrame *parent);                   // @004283f0 slot 3: the frame, then only the child at "level" under it
    void  UpdateLevel();                                                      // @00428510 slot 8: show the child that fits the camera's distance
    int   SelectLevel(Float3 cameraPosition, Float3 cameraForward);           // @004286d0 the level for a camera at that position looking that way
};
static_assert(sizeof(LODProp) == 0xc4, "LODProp layout");
