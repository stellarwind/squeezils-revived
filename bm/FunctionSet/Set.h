// SetToFunction (vtable 0x49e89c) and AddToFunction (0x49e8b4): script variable assignment.
#pragma once
#include "FunctionArgs/FunctionArgs.h"

struct SetArgList {
    virtual String ReadString(int flag);     // slot 0
    virtual void a1();
    virtual void Skip(int n);                // slot 2
};
struct Variables {                           // the global script variables (object at 0x4c7060)
    virtual void Set(String *name, int *value, int flags);   // slot 0
    virtual int Contains(String *name);                      // slot 1
    int Get(String *name);                                       // 0x461230
};
extern Variables g_vars;                                    // 0x4c7060
#define VARS (&g_vars)

class SetToFunction : public Function {
public:
    SetToFunction() { value = 0; }
    ~SetToFunction() { delete value; }
    virtual int Execute(ScriptCall *args, int extra);    // 0x444340
    virtual Function *Create(ArgList *list);             // 0x444130
    virtual void Print();                                // 0x444300
    virtual const char *Name();                          // 0x444230
    String name;                                         // 0x04
    IntFunctionArgument *value;                          // 0x08
};

class AddToFunction : public Function {
public:
    AddToFunction() { value = 0; }
    virtual int Execute(ScriptCall *args, int extra);    // 0x444590
    virtual Function *Create(ArgList *list);             // 0x444380
    virtual void Print();                                // 0x444550
    virtual const char *Name();                          // 0x444480
    String name;                                         // 0x04
    IntFunctionArgument *value;                          // 0x08
};
