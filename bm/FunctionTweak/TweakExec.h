// Stub views of what the TweakFloatFunction commands touch (a creature's tuning record and its default copy).
#pragma once
#include "FunctionTweak/TweakClasses.h"

struct Physics {                          // the tuning record a creature body owns
    char pad[0x1c];
    float frictionA;                      // 0x1c
    float frictionB;                      // 0x20
};
struct TweakActor {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual Physics *Tuning();            // slot 10
    virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual Physics *Defaults();          // slot 20
};
struct CallActor {
    TweakActor *Actor();                  // 0x420c20
    TweakActor *Actor2();                 // 0x420c40
};
struct FloatArg {
    float Get(int context);               // 0x421440
};
#define ARGF(i, extra) (((FloatArg *)args.Get(i))->Get(extra))
