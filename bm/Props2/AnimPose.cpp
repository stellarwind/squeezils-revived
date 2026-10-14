// Spare declarations preserve VC++ 4.1 register allocation for ApplyPose.
void PoseDecl1();
void PoseDecl2();
void PoseDecl3();
void PoseDecl4();
void PoseDecl5();
void PoseDecl6();
void PoseDecl7();
void PoseDecl8();
void PoseDecl9();
void PoseDecl10();
void PoseDecl11();
void PoseDecl12();
void PoseDecl13();
void PoseDecl14();
void PoseDecl15();
void PoseDecl16();
void PoseDecl17();
void PoseDecl18();
void PoseDecl19();
void PoseDecl20();
void PoseDecl21();
void PoseDecl22();
void PoseDecl23();
void PoseDecl24();
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

struct PoseDispatch { void (BodyPartLike::*apply)(ComObject *, void *); };
void AnimatedBody::ApplyPose()
{
    for (int i = 0; i < PARTS_COUNT; i++) {
        BodyPartLike *p = PARTS_DATA[i];
        PoseDispatch *v = *(PoseDispatch **)p;
        (p->*(v->apply))((ComObject *)rootFrame, ((PoseEntries *)pose)->Entry(i));
    }
}
