// Execute slots of ShowProp / HideProp.
#pragma once
#include "FunctionStr/Str.h"

struct PropV {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void Show(int how);                          // slot 4 (0x10)
    virtual void Hide();                                 // slot 5 (0x14)
    virtual void v6(); virtual void v7(); virtual void v8();
    virtual int Shown();                                 // slot 9 (0x24)
    virtual int ShowKind();                              // slot 10 (0x28)
};
struct PropLevel { int Level(); };                       // ScriptCall::Level 0x420c70
PropV *FindPropV(String name, int level, int how);       // 0x42c8b0 (cdecl)
struct Notifier { int NextKind(); };                     // 0x477290
extern Notifier *g_notifier;                             // 0x4c75e0
