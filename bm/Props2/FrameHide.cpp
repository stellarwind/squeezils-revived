// FrameProp::Hide (virtual slot 5, SQZ.EXE 0x427320): hide the children, detach the body, hide the frame, leave the active list.
#define FRAMEPROP_HIDE
#include "FrameProp/FrameProp.h"

struct HideChild { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
                   virtual void Hide(); };               // slot 5 (+0x14)
struct ChildList {                                       // the array object at +0x54
    virtual HideChild *Get(int i);                       // slot 0
    int count;                                           // 0x58
};
struct MeshBase { void Hide(); };                        // 0x426170
#define CHILDREN ((ChildList *)((char *)this + 0x54))
void RemoveFromActive(void *prop);                       // 0x42d1a0 (cdecl)

void FrameProp::Hide()
{
    for (int i = 0; i < *(int *)((char *)this + 0x58); i++)
        if (CHILDREN->Get(i))
            CHILDREN->Get(i)->Hide();
    if (body)
        body->Detach(*(void **)((char *)this + 0x34));
    ((MeshBase *)this)->Hide();
    RemoveFromActive(this);
}
