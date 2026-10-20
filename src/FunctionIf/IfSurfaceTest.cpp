// IfOnSurfaceTypeFunction::Test (0x443b90): resolve a surface binding and compare it with the requested type.
#include <string.h>
#pragma intrinsic(strcmp)
#include "FunctionIf/IfSurface.h"
#include "Ini/IniFile.h"

struct FaceMat {                                         // 64 bytes, empty out-of-line destructor 0x402310
    float a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    FaceMat() {}
    FaceMat(const FaceMat &o) {
        a0 = o.a0;
        a1 = o.a1;
        a2 = o.a2;
        a3 = o.a3;
        a4 = o.a4;
        a5 = o.a5;
        a6 = o.a6;
        a7 = o.a7;
        a8 = o.a8;
        a9 = o.a9;
        a10 = o.a10;
        a11 = o.a11;
        a12 = o.a12;
        a13 = o.a13;
        a14 = o.a14;
        a15 = o.a15;
    }
    ~FaceMat();
};
struct FacePlacement {                                   // 0x48 bytes, destructor 0x4031e0
    int prop;
    int pad;
    FaceMat at;
    FacePlacement() {}
    FacePlacement(const FacePlacement &o) : prop(o.prop), pad(o.pad), at(o.at) {}
    ~FacePlacement();
};
struct FaceTable { String FaceSetName(FacePlacement *where); IniValueTable *Properties(FacePlacement *where); };   // 0x40f3c0
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

int IfOnSurfaceTypeFunction::Test(ScriptCall *args, int e)
{
    if (!extra && !((FaceCall *)args)->Actor()->Body()->OnGround())
        return 0;
    FacePlacement where(*((FaceCall *)args)->Actor()->Body()->Standing());
    String name = GetPlace()->GetSurfaces()->FaceSetName(&where);
    const char *wanted = type->Get(e).c_str();
    String bound = GetPlace()->GetSurfaces()->Properties(&where)->Get(name);
    if (strcmp(bound.c_str(), wanted) == 0)
        return 1;
    return 0;
}
