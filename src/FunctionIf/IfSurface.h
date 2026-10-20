// IfOnSurfaceTypeFunction (vtable 0x49e84c) and the two classes that reuse its operand (IfOnFaceSet 0x49eadc, IfOnOrAboveSurfaceType 0x49eb7c).
#pragma once
#include "FunctionIf/FunctionIf.h"

class StringFunctionArgument : public FunctionArgument {      // vtable 0x49da9c
public:
    StringFunctionArgument();
    virtual void Print();                               // 0x421a70
    String text;                                        // 0x0c
    String Get(int context);                            // 0x421a90 (by value)
};
StringFunctionArgument *ParseStringArgument(ArgList *list);   // 0x421940 (cdecl)

class IfOnSurfaceTypeFunction : public IfFunction {
public:
    IfOnSurfaceTypeFunction() { type = 0; extra = 0; }
    ~IfOnSurfaceTypeFunction();                         // 0x443900
    virtual const char *Name();                         // 0x443a90
    virtual int Parse(ArgList *list);                   // 0x443b60
    virtual void Dump();                                // 0x443b80
    virtual int Test(ScriptCall *args, int extra);      // 0x443b90
    virtual IfFunction *Make();                         // 0x443aa0
    StringFunctionArgument *type;                           // 0x08
    int extra;                                          // 0x0c
};

class IfOnFaceSetFunction : public IfOnSurfaceTypeFunction {
public:
    virtual const char *Name();                         // 0x4462f0
    virtual int Test(ScriptCall *args, int extra);      // 0x443df0
    virtual IfFunction *Make();                         // 0x446300
};

class IfOnOrAboveSurfaceTypeFunction : public IfOnSurfaceTypeFunction {
public:
    virtual const char *Name();                         // 0x446080
    virtual int Test(ScriptCall *args, int extra);      // 0x443d90
    virtual IfFunction *Make();                         // 0x446090
};
