// The CreateProp subclasses (vtables 0x4a0f14, 0x4a0fc4, 0x4a0ec4, 0x4a1014).
#pragma once
#include "FunctionProp/Prop.h"

class CreatePropAtPositionFunction : public CreatePropFunction {
public:
    virtual const char *Name();                          // 0x470f70
    virtual CreatePropFunction *Make();                  // 0x470f80
    virtual Vector3f Position(ScriptCall *args, int extra);   // 0x46fe60
    virtual int ParseExtra(ArgList *list);               // 0x46fdd0
    virtual void DumpExtra();                            // 0x46fe50
    Vector3fFunctionArgument *offset;                    // 0x10
};
class CreatePropAtSqueezilPositionFunction : public CreatePropFunction {
public:
    virtual const char *Name();                          // 0x470bd0
    virtual CreatePropFunction *Make();                  // 0x470be0
    virtual Vector3f Position(ScriptCall *args, int extra);   // 0x46fce0
};
class CreatePropTrackingSqueezilPositionFunction : public CreatePropAtSqueezilPositionFunction {
public:
    virtual int Apply(void *a, void *b, void *c);        // 0x46fd70
    virtual const char *Name();                          // 0x470d30
    virtual CreatePropFunction *Make();                  // 0x470d40
};
class CreatePropTrackingSqueezilXformFunction : public CreatePropAtSqueezilPositionFunction {
public:
    virtual int Apply(void *a, void *b, void *c);        // 0x46fda0
    virtual const char *Name();                          // 0x470e50
    virtual CreatePropFunction *Make();                  // 0x470e60
};
