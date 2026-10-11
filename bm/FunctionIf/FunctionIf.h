// IfFunction (vtable 0x49e874) and its subclasses: `If<condition>` runs the next script command only when the condition holds.
#pragma once
#include "FunctionArgs/FunctionArgs.h"

struct ExecContext {                       // 0x18 bytes: what a deferred or nested command needs from the caller's record
    ExecContext(ScriptCall *call);         // 0x420bf0
    char data[0x18];
};
Function *ParseFunction(ArgList *list);    // 0x421f50 (cdecl): reads the next command of the script text

class IfFunction : public Function {
public:
    IfFunction() { child = 0; }
    ~IfFunction() { delete child; }                     // 0x443a10 (scalar deleting)
    virtual int Execute(ScriptCall *args, int extra);   // slot 1: 0x443750
    virtual Function *Create(ArgList *list);            // slot 2: 0x4436e0
    virtual void Print();                               // slot 3: 0x443720
    virtual int Parse(ArgList *list) = 0;               // slot 5: read the condition's own operands
    virtual void Dump() = 0;                            // slot 6: print them
    virtual int Test(ScriptCall *args, int extra) = 0;  // slot 7: is the condition true?
    virtual IfFunction *Make() = 0;                     // slot 8: a fresh instance
    Function *child;                                    // 0x04 the command to run
};
