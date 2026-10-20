// IfVariableFunction (vtable 0x49ea5c), IfVariableValueFunction (0x49e9fc) and Equal/Lesser/Greater (0x49e96c/0x49e99c/0x49e9cc).
#pragma once
#include "FunctionIf/FunctionIf.h"

struct ArgList {
    virtual String ReadString(int flag);          // slot 0
    virtual void a1();
    virtual void Skip(int n);                            // slot 2
};

struct VariableTable {                  // the global script variables (object at 0x4c7060)
    virtual void v0();
    virtual int Contains(String *name);          // slot 1
    int Get(String *name);                               // 0x461230
};
extern VariableTable g_variables;       // 0x4c7060

class IfVariableFunction : public IfFunction {
public:
    IfVariableFunction() {}
#ifdef IFVAR_DTORS
    ~IfVariableFunction() {}                             // 0x4464b0 (scalar deleting 0x446520); only IfVarDtor.cpp defines it: in the other units it moves registers
#endif
    virtual int Parse(ArgList *list);                    // 0x443fb0
    virtual void Dump();                                 // 0x444040
    virtual int Test(ScriptCall *args, int extra);       // 0x444060
    virtual int Check(int value, ScriptCall *args, int extra) = 0;   // slot 9
    String name;                                         // 0x08
};

class IfVariableValueFunction : public IfVariableFunction {
public:
    IfVariableValueFunction() { value = 0; }
#ifdef IFVAR_DTORS
    ~IfVariableValueFunction() { delete value; }         // 0x446700
#endif
    virtual int Parse(ArgList *list);                    // 0x4440b0
    virtual void Dump();                                 // 0x4440e0
    virtual int Check(int v, ScriptCall *args, int extra);   // 0x444100
    virtual int Compare(int v, int ref, ScriptCall *args) = 0;   // slot 10
    IntFunctionArgument *value;                          // 0x0c
};

class IfVariableEqualFunction : public IfVariableValueFunction {
public:
    virtual const char *Name();                          // 0x446b10
    virtual IfFunction *Make();                          // 0x446b20
    virtual int Compare(int v, int ref, ScriptCall *args);   // 0x446b00
};

class IfVariableLesserFunction : public IfVariableValueFunction {
public:
    virtual const char *Name();                          // 0x4469a0
    virtual IfFunction *Make();                          // 0x4469b0
    virtual int Compare(int v, int ref, ScriptCall *args);   // 0x446990
};

class IfVariableGreaterFunction : public IfVariableValueFunction {
public:
    virtual const char *Name();                          // 0x446830
    virtual IfFunction *Make();                          // 0x446840
    virtual int Compare(int v, int ref, ScriptCall *args);   // 0x446820
};
