// ExecuteWithPropFunction (vtable 0x49e8fc): run the next command with a named prop as its context.
#pragma once
#include "FunctionIf/FunctionIf.h"

struct PropArgList {
    virtual String ReadString(int flag);                 // slot 0
    virtual void a1();
    virtual void Skip(int n);                            // slot 2
};
struct PropCall { int Level(); };                        // ScriptCall::Level 0x420c70
void *FindProp(String name, int level, int zero);        // 0x42c8b0 (cdecl)

class ExecuteWithPropFunction : public Function {
public:
    ExecuteWithPropFunction() { child = 0; }
    ~ExecuteWithPropFunction() { delete child; }         // 0x444b00 sdd
    virtual int Execute(ScriptCall *args, int extra);    // 0x444bc0
    virtual Function *Create(ArgList *list);             // 0x4449e0
    virtual void Print();                                // 0x444b90
    virtual const char *Name();                          // 0x444af0
    String name;                                         // 0x04
    Function *child;                                     // 0x08
};
