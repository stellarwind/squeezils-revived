// CreatePropFunction (vtable 0x4a0e3c): base of the "create a prop" commands (name, position, direction).
#pragma once
#include "FunctionExec/Exec.h"
#include "Vector3f/Vector3f.h"

class Vector3fFunctionArgument : public FunctionArgument {   // vtable 0x49da94
public:
    Vector3fFunctionArgument();
    virtual void Print();                                // 0x4217c0
    Vector3f value;                                      // 0x0c
};
Vector3fFunctionArgument *ParseVectorArgument(ArgList *list, Vector3f fallback);   // 0x421560 (cdecl)

class CreatePropFunction : public Function {
public:
    CreatePropFunction();                                // 0x46f800
    ~CreatePropFunction();                               // 0x46f8c0
    virtual int Execute(ScriptCall *args, int extra);    // 0x46fb00
    virtual Function *Create(ArgList *list);             // 0x46f9e0
    virtual void Print();                                // 0x46fab0
    virtual CreatePropFunction *Make() = 0;              // slot 5
    virtual Vector3f Position(ScriptCall *args, int extra) = 0;   // slot 6
    virtual int ParseExtra(ArgList *list);               // slot 7: 0x46f870 returns 1
    virtual void DumpExtra();                            // slot 8: 0x46f880
    virtual int Apply(void *a, void *b, void *c);        // slot 9: 0x46f890
    StringFunctionArgument *name;                        // 0x04
    Vector3fFunctionArgument *position;                  // 0x08
    Vector3fFunctionArgument *direction;                 // 0x0c
};
