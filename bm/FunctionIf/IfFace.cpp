// IfOnFaceSetFunction::Test (0x443df0): is the creature standing on a face of the named face set?
#include <string.h>
#pragma intrinsic(strcmp)
#include "FunctionIf/IfSurface.h"

struct FaceMat {                                         // 64 bytes, empty out-of-line destructor 0x402310
    float a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    FaceMat() {}
    ~FaceMat();
};
struct FacePlacement {                                   // 0x48 bytes, destructor 0x4031e0
    int prop;
    int pad;
    FaceMat at;
    FacePlacement() {}
    FacePlacement(const FacePlacement &o) : prop(o.prop), pad(o.pad)
    {
        at.a0 = o.at.a0; at.a1 = o.at.a1; at.a2 = o.at.a2; at.a3 = o.at.a3;
        at.a4 = o.at.a4; at.a5 = o.at.a5; at.a6 = o.at.a6; at.a7 = o.at.a7;
        at.a8 = o.at.a8; at.a9 = o.at.a9; at.a10 = o.at.a10; at.a11 = o.at.a11;
        at.a12 = o.at.a12; at.a13 = o.at.a13; at.a14 = o.at.a14; at.a15 = o.at.a15;
    }
    ~FacePlacement();
};
struct FaceTable { String FaceSetName(FacePlacement *where); };   // 0x40f3c0
struct FacePlace { FaceTable *GetSurfaces(); };          // 0x439b60
FacePlace *GetPlace();                                   // 0x422bd0 (cdecl)
struct FaceBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6();
    virtual FacePlacement *Standing();                   // slot 7 (+0x1c)
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11(); virtual void v12();
    virtual void v13(); virtual void v14();
    virtual int OnGround();                              // slot 15 (+0x3c)
};
struct FaceActor {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17();
    virtual void v18(); virtual void v19();
};
struct FaceActorBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16();
    virtual FaceBody *Body();                            // slot 17 (+0x44)
};
struct FaceCall { FaceActorBody *Actor(); };             // 0x420c20

int IfOnFaceSetFunction::Test(ScriptCall *args, int e)
{
    if (!((FaceCall *)args)->Actor()->Body()->OnGround())
        return 0;
    FacePlacement where(*((FaceCall *)args)->Actor()->Body()->Standing());
    String name = GetPlace()->GetSurfaces()->FaceSetName(&where);
    const char *wanted = type->Get(e).c_str();
    if (strcmp(name.c_str(), wanted) == 0)
        return 1;
    return 0;
}
