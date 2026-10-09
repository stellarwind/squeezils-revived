// SqueezilSoul::IsAttacking (0x4509d0): state==ATTACK, or FLY with the cannon clip. Virtual-call leaf.
#pragma once
#include "compat41.h"

struct IsAttackingBody {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual int GetMode();   // slot 20
};

struct IsAttackingSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual IsAttackingBody *GetBody();  // slot 17
    char pad[0x38];
    int  state;   // 0x3c

    int IsAttacking();   // 0x4509d0
};
