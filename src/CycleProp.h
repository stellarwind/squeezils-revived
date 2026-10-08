// CycleProp: a FrameProp that shows its children one after another, like the pages of a flip book: one child at a time, the next
// one every "period" seconds. Reconstructed from SQZ.EXE 0x4288b0-0x429380 (class name and vtable 0x49dd44 from RTTI). Hide,
// FindChild and the cycle setters are FrameProp's.
//
// How it steps: "step" is +1 or -1 (0 stops it). Each time the timer runs out the current child number moves by step. Past
// either end, "mode" decides: 1 wraps round to the other end, -1 turns round and goes back, any other value stops at the end
// (and hides the prop when "hide at end" is set). The timer is topped up by the period ("carry" set: any time over is kept) or
// reset to it.
//
// Step events: a table from a child number to the name of a script function. Whenever the current number goes past one of those
// numbers (including by wrapping), the function runs with the prop as the caller. See FireStepMessages for the exact range.
//
// CycleProp's own vtable slots: 0 Destroy  1 Clone  3 BuildFrame  7 Update(seconds)  22 SetCurrent(child)  23 SetStep  24 SetMode
#pragma once
#include "FrameProp.h"

typedef BufferObjectTable<int, String, StringList, 24> StepEventTable;

enum CyclePropSlot { CYCLEPROP_SET_CURRENT = 22, CYCLEPROP_SET_STEP = 23, CYCLEPROP_SET_MODE = 24 };

struct CycleProp : FrameProp {
    StepEventTable events;      // 0xac child number -> script function name (0x30 bytes)
    int            current;     // 0xdc the child on show
    int            step;        // 0xe0 +1 or -1 (0 stops it)
    int            mode;        // 0xe4 at an end: 1 wrap round, -1 turn back, else stop
    int            carry;       // 0xe8 nonzero: time left over when the timer runs out is kept (the "UseRealTime" key of the prop's section)
    float          timer;       // 0xec seconds until the next step
    float          period;      // 0xf0 seconds between steps

    CycleProp *Init(Float3 position, Float3 scale, Float3 direction, Float3 up, StringList *childNames, MorphCycle *cycle,
                    CycleMessageTable *messages, StepEventTable *events, float period, int carry, float startTime);   // @004288b0
    void  Destruct();                                                         // @00428d10
    Prop *Destroy(int flags);                                                 // @00428b60 slot 0
    Prop *Clone(FrameProp *owner);                                            // @00428bd0 slot 1
    IDirect3DRMFrame *BuildFrame(IDirect3DRMFrame *parent);                   // @00428d80 slot 3: the frame, then the current child under it
    void  Update(float seconds);                                              // @00428e90 slot 7: FrameProp's update, then the stepping
    void  SetCurrent(int child);                                              // @00428fd0 slot 22 (@00429370 is its thin wrapper): swaps the child on show
    void  SetStep(int newStep);                                               // @00429350 slot 23
    void  SetMode(int newMode);                                               // @00429360 slot 24
    void  FireStepMessages(int from, int to);                                 // @004290b0 runs the events for the numbers passed going from -> to
};
static_assert(sizeof(CycleProp) == 0xf4, "CycleProp layout");
