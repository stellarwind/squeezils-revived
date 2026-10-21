// FunctionArgument family (vtables 0x49da74-0x49dae0): how a script command reads one operand.
// literal != 0: the value is stored in the object (+0xc); otherwise `name` is a script variable looked up at run time.
#pragma once
#include "compat41.h"
#include "String/String.h"

class FunctionArgument {
public:
    FunctionArgument();                      // 0x420d20
    FunctionArgument(const char *variable);  // 0x420c80
    virtual void Print();                    // slot 0: 0x420d90 for the base
    // no user dtor: the original 0x420e20 is the implicit one (no vptr store); declaring ~FunctionArgument() adds one
    int    literal;                          // 0x04
    String name;                             // 0x08
};

class IntFunctionArgument : public FunctionArgument {
public:
    int value;                               // 0x0c
    IntFunctionArgument(int v);              // 0x420db0
    IntFunctionArgument(const char *variable);   // 0x420e70
    virtual void Print();                    // 0x421130
    int Get(int context);                    // 0x421150
};

// ---- the Function base (RTTI vtable 0x49dabc) ----
struct ArgList;                          // what a script command's text was parsed into (0x420ee0 reads it)
FunctionArgument *ParseArgument(ArgList *list);   // 0x420ee0 (cdecl)
int DebugLogAll(const char *fmt, ...);     // 0x419290

struct ScriptCall {                      // the small record PostFunction passes as `args`
    int pad[2];                          // 0x00
    struct Target *target;               // 0x08
    struct Target *Actor();              // 0x420c20 (not inlined)
};

class Function {
public:
    virtual ~Function();                 // slot 0 (0x422ea0 for the base)
    virtual int Execute(ScriptCall *args, int extra) = 0;   // slot 1
    virtual Function *Create(ArgList *list) = 0;            // slot 2
    virtual void Print() = 0;            // slot 3
    virtual const char *Name() = 0;      // slot 4
};
