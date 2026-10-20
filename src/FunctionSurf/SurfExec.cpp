// SetSurfaceTypeFunction::Execute (0x440a30): hand the (name, type) pair and the calling prop's handle to the place's surface table.
#include "FunctionSurf/Surf.h"

struct SurfMatrix {                                      // 64-byte matrix with an empty out-of-line destructor (0x402310)
    float m[16];
    SurfMatrix() {}
    ~SurfMatrix();
};
struct SurfPlacement {                                   // what the surface table receives for the calling prop (dtor 0x4031e0)
    int prop;                                            // 0x00
    int pad;                                             // 0x04
    SurfMatrix at;                                       // 0x08
    SurfPlacement() { prop = -1; pad = 0; }
    ~SurfPlacement();
};
struct SurfTable { void SetType(String name, String type, SurfPlacement *where); };   // 0x40f450
struct SurfPlace { SurfTable *GetSurfaces(); };          // 0x439b60
SurfPlace *GetPlace();                                   // 0x422bd0 (cdecl)
struct SurfCall { void *Level(); };                      // 0x420c70
struct SurfProp {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
    virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25();
    virtual int Handle();                                // slot 26 (+0x68)
};
void *DynamicCast(void *p, int zero, const void *from, const void *to, int ref);   // 0x48f060 (cdecl)
extern const char kPropType[];                           // 0x4bb910
extern const char kHandleType[];                         // 0x4bbb58

int SetSurfaceTypeFunction::Execute(ScriptCall *args, int extra)
{
    SurfPlacement where;
    if (((SurfCall *)args)->Level())
        where.prop = ((SurfProp *)DynamicCast(((SurfCall *)args)->Level(), 0, kPropType, kHandleType, 0))->Handle();
    else
        where.prop = 0;
    GetPlace()->GetSurfaces()->SetType(first->Get(extra), second->Get(extra), &where);
    return 1;
}
