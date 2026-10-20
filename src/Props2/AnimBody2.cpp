// 5 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
// AnimatedBody virtual slots 1, 2, 4 (SQZ.EXE 0x41dda0, 0x41ddf0, 0x41df10): bind the parts to a scene frame, release it, apply the pose.
#define ANIMBODY_SLOTS
#include "AnimatedBody/AnimatedBody.h"

struct ComObject {                                       // a Direct3D retained-mode object, seen through IUnknown
    virtual long __stdcall QueryInterface(void *iid, void **out);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
};
struct SceneFrames { ComObject *Lookup(int name); };     // 0x476f90 on the scene table
extern SceneFrames *g_scene;                             // 0x4c75e0
struct BodyPartLike {
    virtual void Apply(ComObject *frame, void *entry);   // slot 0
    virtual void Bind(ComObject *frame);                 // slot 1
};
struct PartArray {                                       // the part table inside the body (starts at +4)
    int pad[2];
    int count;                                           // body +0x0c
    int pad2[6];
    BodyPartLike **data;                                 // body +0x28
};
#define PARTS_COUNT (*(int *)((char *)this + 0x0c))
#define PARTS_DATA (*(BodyPartLike ***)((char *)this + 0x28))
struct PoseEntries { void *Entry(int i); };              // 0x41b750
struct BodyOwner {                                       // whoever the body is detached from (virtual slot 38)
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual void v25(); virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29();
    virtual void v30(); virtual void v31(); virtual void v32(); virtual void v33(); virtual void v34();
    virtual void v35(); virtual void v36(); virtual void v37();
    virtual long __stdcall DeleteChild(ComObject *frame);   // slot 38 (+0x98): IDirect3DRMFrame::DeleteChild
};

void AnimatedBody::Attach(int frameName)
{
    ComObject *root = g_scene->Lookup(frameName);
    rootFrame = root;
    if (root)
        root->AddRef();
    for (int i = 0; i < PARTS_COUNT; i++)
        PARTS_DATA[i]->Bind((ComObject *)rootFrame);
}

void AnimatedBody::Detach(void *owner)
{
    ((BodyOwner *)owner)->DeleteChild((ComObject *)rootFrame);
    ((ComObject *)rootFrame)->Release();
    rootFrame = 0;
}
