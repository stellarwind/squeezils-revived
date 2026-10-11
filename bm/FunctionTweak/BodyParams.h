// The tuning record behind a creature body (0x2a0 bytes) and the actor view the Tweak commands use. Shared by exec/Exec_*.cpp.
#pragma once
#include "FunctionTweak/TweakExec.h"
#include "Vector3f/Vector3f.h"

struct Mat9 {                             // 0x24 bytes: two vectors (their empty destructors are the EH funclets) and three floats
    Vector3f u, v;
    float w0, w1, w2;
    Mat9 &operator=(const Mat9 &o);      // 0x435760
};
struct B24 { float v[6]; };
struct B92 { float v[23]; };
struct B40 { float v[10]; };
struct B44 { float v[11]; };
struct BodyParams {
    Mat9 a;                               // 0x000
    B24 b0, b1, b2, b3;                   // 0x024
    B92 c0, c1, c2;                       // 0x084
    B40 d0, d1, d2;                       // 0x198
    B44 e0, e1;                           // 0x210
    float f0, f1, f2, f3, f4, f5, f6, f7; int i8; float f9, f10, f11, f12, f13;   // 0x268
};
struct ParamActor {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual BodyParams *Defaults();       // slot 20
    char pad[0x48 - 4];
    BodyParams *params;                   // 0x48
};
