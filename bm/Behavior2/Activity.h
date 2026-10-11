// Activity / behaviour objects of the robo squeezils (SQZ.EXE 0x401700-0x404200). Layout learned from the Execute bodies.
#pragma once
#include "compat41.h"

struct ActPlace;
struct ActLevel {
    void DropHeld(void *inventory);                      // 0x44e0b0
};
struct ActPlaceRef { ActLevel *GetLevel(); };            // 0x439b50
struct ActInventory { void Clear(); };                   // 0x4164a0
struct ActOwner { ActInventory *Inventory(); };          // 0x448990
struct ActSoul {                                         // the creature the activity belongs to
    char pad[0x6c];
    ActPlaceRef *place;                                  // 0x70 (the vptr takes 0)
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17();
    virtual ActOwner *Owner();                           // slot 18 (+0x48)
};

struct Activity {                                        // base of the activities (dtor 0x405200 is shared by Die/Regen)
    virtual ~Activity();
    virtual int Execute(int dt) = 0;                     // slot 1
    void *body;                                          // 0x04
    ActSoul *soul;                                       // 0x08
};
