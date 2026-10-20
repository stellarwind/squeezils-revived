// FrameProp: a group of child props under a frame (SQZ.EXE 0x426c70-0x427f60). Leaf port: the three guarded
// cycle setters (0x427e30/0x427e50/0x427e70, virtual calls into AnimatedBody), the three trivial field setters
// (0x427f10/0x427f20/0x427f30), HasBody (0x427f40) and ChildNames (0x427f50). childNames at +0x40, body at +0x94.
#pragma once
#include "compat41.h"

// AnimatedBody vtable as seen from the callers: slot 6 SetDirection, slot 7 SetLoopMode, slot 14 SetTime.
struct AnimatedBody {
    virtual void s0(); virtual void s1(); virtual void Detach(void *owner); virtual void s3();   // slot 2 = Detach (0x41ddf0)
    virtual void s4(); virtual void s5();
    virtual void SetDirection(int d);   // slot 6
    virtual void SetLoopMode(int m);    // slot 7
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13();
    virtual void SetTime(float t);      // slot 14
};

struct FrameProp {
    char pad40[0x40];          // 0x00 Prop base
    char childNames[4];        // 0x40 (StringList)
    char pad44[0x94 - 0x44];   // 0x44 children..messages
    AnimatedBody *body;        // 0x94
    char pad98[0x9c - 0x98];   // 0x98 cycle
    int  actorIndex;           // 0x9c
    int  followMode;           // 0xa0
    char pada4[0xa8 - 0xa4];   // 0xa4 startTime
    int  hideAtEnd;            // 0xa8

    void *ChildNames();                    // 0x427f50 slot 12
    void  SetCycleTime(float time);        // 0x427e30 slot 15
    void  SetCycleDirection(int d);        // 0x427e50 slot 16
    void  SetCycleLoopMode(int mode);      // 0x427e70 slot 17
    void  SetFollowMode(int mode);         // 0x427f20 slot 19
    void  SetActor(int index);             // 0x427f10 slot 20
    void  SetHideAtEnd(int hide);          // 0x427f30 slot 21
    int   HasBody();                       // 0x427f40 (not virtual)
    void FireTimeMessages(float from, float to); // 0x427b50
    void Update(float seconds);                 // 0x427740
#ifdef FRAMEPROP_BUILD_OWN
    void BuildOwnFrame(void *parent);           // 0x427380
#endif
#ifdef FRAMEPROP_HIDE
    void Hide();                           // slot 5  0x427320
#endif
};
