// Slots 7-9 of the CreateProp subclasses (SQZ.EXE 0x46fd70-0x46fe90).
#define V3_CTOR_OUTOFLINE
#include "FunctionProp/PropClasses.h"

struct ApplyCall { int Other(); };                       // 0x420c60
struct ApplyProp {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
    virtual void v18();
    virtual int Mode(int how);                           // slot 19 (+0x4c)
    virtual void Follow(int on);                         // slot 20 (+0x50)
};
int CreatePropTrackingSqueezilXformFunction::Apply(void *a, void *b, void *c)
{
    ApplyProp *p = (ApplyProp *)a;
    p->Follow(((ApplyCall *)b)->Other());
    return p->Mode(2);
}
