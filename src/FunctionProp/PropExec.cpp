// CreatePropFunction::Execute (0x46fb00, shared by the four subclasses): find the prop template by name, make one at the computed position.
#define V3_CTOR_OUTOFLINE
#define V3_COPYCTOR_OUTOFLINE
#include "FunctionProp/Prop.h"

struct PropTemplate {                                    // what FindProp returns
    virtual void v0(); virtual void v1();
    virtual void *Make(int flag, Vector3f pos, Vector3f scale, Vector3f positionArg, Vector3f directionArg);   // slot 2 (+8)
};
struct PropObject {                                      // the made prop, seen as a generic prop
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void Attach(void *parent);                   // slot 4 (+0x10)
};
struct PropAnim {                                        // the CycleProp view of it
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16();
    virtual void Reset(int a);                           // slot 17 (+0x44)
    virtual void v18(); virtual void v19(); virtual void v20();
    virtual void Start(int a);                           // slot 21 (+0x54)
};
struct PropFramed {                                      // the FrameProp view of it
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
    virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void SetWrap(int a);                         // slot 24 (+0x60)
};
struct SceneHolder { void *Child(); };                   // 0x477290
extern SceneHolder *g_scene;                             // 0x4c75e0
PropTemplate *FindPropTemplate(String name, int level, int how);   // 0x42c8b0 (cdecl)
void *DynamicCast(void *p, int zero, const void *from, const void *to, int ref);   // 0x48f060 (cdecl)
void DestroyPropLater(void *prop);                       // 0x42d1f0 (cdecl)
extern const char kPropType[];                           // 0x4bb910
extern const char kCyclePropType[];                      // 0x4bb9c0
extern const char kFrameType[];                          // 0x4bbb40

int CreatePropFunction::Execute(ScriptCall *args, int extra)
{
    PropTemplate *proto = FindPropTemplate(name->Get(extra), 0, 0);
    if (!proto)
        return 1;
    Vector3f pos = Position(args, extra);
    void *obj = proto->Make(0, pos, Vector3f(1, 1, 1), position->Get(extra), direction->Get(extra));
    PropAnim *anim = (PropAnim *)DynamicCast(obj, 0, kPropType, kCyclePropType, 0);
    if (!anim)
        return 1;
    DestroyPropLater(anim);
    ((PropObject *)obj)->Attach(g_scene->Child());
    anim->Reset(0);
    anim->Start(1);
    Apply(anim, args, (void *)extra);
    PropFramed *framed = (PropFramed *)DynamicCast(obj, 0, kPropType, kFrameType, 0);
    if (framed)
        framed->SetWrap(0);
    return 1;
}
