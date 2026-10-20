// SetCyclePropFunction (vtable 0x4a0f64) and its six subclasses: set one property of a named animated prop to a float.
#pragma once
#include "FunctionArgs/FunctionArgs.h"
#include "FunctionMisc/MiscClasses.h"

struct ArgList2 {                        // the argument list as the CycleProp commands read it
    virtual String ReadString(int flag);       // slot 0
    virtual void a1();
    virtual void Skip(int n);                  // slot 2
};
struct CycleProp {                       // the animated prop found by name
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
    virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void SetWrap(int);                 // slot 24 (+0x60)
};
void *FindProp(String name, int level, int zero);        // 0x42c8b0 (cdecl)
void *DynamicCast(void *p, int zero, const void *from, const void *to, int ref);   // 0x48f060 (cdecl)
extern const char kPropType[];           // 0x4bb910
extern const char kCyclePropType[];      // 0x4bb9c0

class SetCyclePropFunction : public Function {
public:
    SetCyclePropFunction() { value = 0; }
    ~SetCyclePropFunction() { delete value; }            // 0x471100; also expanded in scalar deleting dtor
    virtual int Execute(ScriptCall *args, int extra);    // 0x470440
    virtual Function *Create(ArgList *list);             // 0x470360
    virtual void Print();                                // 0x470400
    virtual int Apply(CycleProp *prop, ScriptCall *args, int extra) = 0;   // slot 5
    virtual SetCyclePropFunction *Make() = 0;            // slot 6
    String name;                                         // 0x04
    FloatFunctionArgument *value;                        // 0x08
};

struct CallLevel {                       // ScriptCall seen through its level accessor
    int Level();                         // 0x420c70
};
