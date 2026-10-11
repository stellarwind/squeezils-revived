// ChangeRoboResponseFunction (vtable 0x49dafc): change how a robot answers to a stimulus.
#pragma once
#include "FunctionExec/Exec.h"

class KindArgument : public FunctionArgument {      // names a robot kind
public:
    int value;                           // 0x0c
    int Resolve(int context);            // 0x4235b0
};
KindArgument *ParseKindArgument(ArgList *list);          // 0x423490 (cdecl)

class ChangeRoboResponseFunction : public Function {
public:
    ChangeRoboResponseFunction() { kind = 0; response = 0; }
    ~ChangeRoboResponseFunction() { delete kind; delete response; }
    virtual int Execute(ScriptCall *args, int extra);    // 0x423a50
    virtual Function *Create(ArgList *list);             // 0x423860
    virtual void Print();                                // 0x423a10
    virtual const char *Name();                          // 0x423920
    KindArgument *kind;                                  // 0x04
    StringFunctionArgument *response;                    // 0x08
};

class BehaviorTypeArgument : public FunctionArgument {   // names a robot behaviour (vtable 0x49dbxx)
public:
    int Resolve(int context);                            // 0x4233f0
};
BehaviorTypeArgument *ParseBehaviorTypeArgument(ArgList *list);   // 0x4232c0 (cdecl)

class SetRoboBehaviorFunction : public Function {        // vtable 0x49dae4
public:
    SetRoboBehaviorFunction() { behavior = 0; }
    ~SetRoboBehaviorFunction();                          // 0x4235c0 (out of line); slot 0 0x423650 is the scalar deleting form
    virtual int Execute(ScriptCall *args, int extra);    // 0x423800
    virtual Function *Create(ArgList *list);             // 0x423670
    virtual void Print();                                // 0x4237d0
    virtual const char *Name();                          // 0x423640
    BehaviorTypeArgument *behavior;                      // 0x04
    float values[3];                                     // 0x08
};
