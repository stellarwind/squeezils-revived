// AnimatedBody::ApplyPose (virtual slot 4, SQZ.EXE 0x41df10): push the current pose to every part.
#define ANIMBODY_SLOTS
#include "AnimatedBody/AnimatedBody.h"

struct ComObject {
    virtual long __stdcall QueryInterface(void *iid, void **out);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
};
struct BodyPartLike {
    virtual void Apply(ComObject *frame, void *entry);   // slot 0
    virtual void Bind(ComObject *frame);                 // slot 1
};
#define PARTS_COUNT (*(int *)((char *)this + 0x0c))
#define PARTS_DATA (*(BodyPartLike ***)((char *)this + 0x28))
struct PoseEntries { void *Entry(int i); };              // 0x41b750

void AnimatedBody::ApplyPose()
{
    for (int i = 0; i < PARTS_COUNT; i++) {
        BodyPartLike *p = PARTS_DATA[i];
        p->Apply((ComObject *)rootFrame, ((PoseEntries *)pose)->Entry(i));
    }
}
