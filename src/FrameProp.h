// FrameProp: a prop that is a group of other props. It has a frame of its own (placed like a mesh prop's) and puts its children
// under it, each one a copy of the prop template with that name from the PropList. It can also play an animation clip on a
// one-part body under its frame, and runs script functions at set times in the clip. Reconstructed from SQZ.EXE 0x426c70-0x427f60
// (class name and vtable 0x49dc24 from RTTI). LODProp, CycleProp and NavProp derive from it and are still original code.
//
// Following an actor: with an actor index set (the index of an actor in the current simulation, -1 for none) and a follow mode,
// Update places the frame on that actor every frame: mode 2 turns and moves it with the actor's whole matrix, any other
// nonzero mode only moves it to the actor's position.
//
// The clip messages: a table of (time, function name). When the clip's time passes one of the times, Update runs the script
// function of that name (Script.h: PostFunction), with a record that names this prop and its actor as the caller. Looping clips
// fire across the wrap; a clip that stops at its end hides the prop there when "hide at end" is set.
//
// FrameProp vtable slots (0-14 as in Prop.h; 2, 4, 6, 8-11 and 14 are Prop's):
//     0 Destroy  1 Clone  3 BuildFrame  5 Hide  7 Update(seconds)  12 ChildNames  13 FindChild(name)  15 SetCycleTime
//     16 SetCycleDirection  17 SetCycleLoopMode  18 GetPartMatrix  19 SetFollowMode  20 SetActor  21 SetHideAtEnd
#pragma once
#include "Prop.h"
#include "Container.h"

struct MorphCycle;
struct AnimatedBody;

// The clip messages: time in the clip (seconds) -> name of the script function to run then.
typedef BufferObjectTable<float, String, StringList, 25> CycleMessageTable;

// The record a clip message hands the script function (Script.h: PostFunction's args): who runs it. 0x18 bytes, built by one of
// two original constructors. Only the fields below are known; the others are always 0 from here.
struct ScriptCaller {
    int   unknown0;                 // 0x00
    int   unknown4;                 // 0x04
    void *soul;                     // 0x08 the actor's SqueezilSoul (NULL when the actor is not one)
    int   unknownC;                 // 0x0c
    int   actorIndex;               // 0x10 -1 for none
    Prop *prop;                     // 0x14
};
static_assert(sizeof(ScriptCaller) == 0x18, "ScriptCaller layout");

enum FramePropSlot {
    FRAMEPROP_UPDATE = 7, FRAMEPROP_CHILD_NAMES = 12, FRAMEPROP_FIND_CHILD = 13, FRAMEPROP_SET_CYCLE_TIME = 15,
    FRAMEPROP_SET_CYCLE_DIRECTION = 16, FRAMEPROP_SET_CYCLE_LOOP_MODE = 17, FRAMEPROP_GET_PART_MATRIX = 18,
    FRAMEPROP_SET_FOLLOW_MODE = 19, FRAMEPROP_SET_ACTOR = 20, FRAMEPROP_SET_HIDE_AT_END = 21
};

struct FrameProp : Prop {
    StringList        childNames;   // 0x40 names of the child prop templates
    Array<Prop *>     children;     // 0x54 one per name; NULL until BuildFrame makes the copy of the template
    CycleMessageTable messages;     // 0x64 (0x30 bytes)
    AnimatedBody     *body;         // 0x94 made by BuildFrame when there is a clip
    MorphCycle       *cycle;        // 0x98 the clip the body plays, or NULL
    int               actorIndex;   // 0x9c the actor this prop follows and names as the caller of its messages, -1 for none
    int               followMode;   // 0xa0 0 off, 2 the actor's whole matrix, other values its position only
    float             startTime;    // 0xa4 where the body starts in the clip (0; nothing in FrameProp's code sets it)
    int               hideAtEnd;    // 0xa8 hide the prop when a clip that does not loop reaches its end

    FrameProp *Init(Float3 position, Float3 scale, Float3 direction, Float3 up, StringList *childNames, MorphCycle *cycle,
                    CycleMessageTable *messages);                             // @00426c70 copies the names and the message table
    void  Destruct();                                                         // @00427100 destroys the children and the body
    Prop *Destroy(int flags);                                                 // @00426f70 slot 0
    Prop *Clone(FrameProp *owner);                                            // @00426fe0 slot 1
    IDirect3DRMFrame *BuildFrame(IDirect3DRMFrame *parent);                   // @004271f0 slot 3: own frame, then the children under it
    void  Hide();                                                             // @00427320 slot 5: the children, the body's frames, then itself
    void  Update(float seconds);                                              // @00427740 slot 7: follows the actor, plays the clip, fires its messages
    StringList *ChildNames() { return &childNames; }                          // @00427f50 slot 12 (4 bytes long: stays original)
    Prop *FindChild(const String &name);                                      // @00427f60 slot 13: the child with that name (any case), or NULL
    void  SetCycleTime(float time);                                           // @00427e30 slot 15
    void  SetCycleDirection(int direction);                                   // @00427e50 slot 16
    void  SetCycleLoopMode(int mode);                                         // @00427e70 slot 17
    float *GetPartMatrix(float *result);                                      // @00427e90 slot 18: the body part's pose as a 4x4 matrix
    void  SetFollowMode(int mode);                                            // @00427f20 slot 19
    void  SetActor(int index);                                                // @00427f10 slot 20
    void  SetHideAtEnd(int hide);                                             // @00427f30 slot 21

    void  BuildOwnFrame(IDirect3DRMFrame *parent);                            // @00427380 the frame, the body and the missing children
    IDirect3DRMFrame *GetPartFrame();                                         // @004276b0 the body part's frame, or the prop's frame without a body
    void  FireMessages(float from, float to);                                 // @00427b50 runs the messages whose times the clip passed going from -> to
    void  RunMessage(int index);                                              // inlined twice in FireMessages
    void  RunScript(const char *functionName);                                // inlined wherever a prop runs a script function as the caller (FireMessages, CycleProp::FireStepMessages)
};
static_assert(sizeof(FrameProp) == 0xac, "FrameProp layout");
