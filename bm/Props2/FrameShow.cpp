// FrameProp::Show (virtual slot 3, SQZ.EXE 0x4271f0): show the frame, join the active list, attach every part.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "String/String.h"

struct FramePart {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void *Attach(void *object);                  // slot 4 (+0x10)
    virtual void v5();
    virtual void Place(void *attached, Vector3f scale);  // slot 6 (+0x18)
};
struct FrameNames {                                      // embedded at +0x40
    virtual String Get(int i);                           // slot 0
    int count;
};
struct FrameParts {                                      // embedded at +0x54
    virtual FramePart *Get(int i);                       // slot 0
    int count;
};
struct FrameShow { void *Show(void *parent); };          // 0x427380
void AddActive(void *prop);                              // 0x42d180 (cdecl)

#define PARTS ((FrameParts *)((char *)this + 0x54))
struct FrameProp {
    virtual ~FrameProp();
    virtual FrameProp *Clone(void *owner);               // slot 1
    virtual void v2();
    virtual void *Show(void *parent);                    // slot 3  0x4271f0
    Vector3f position;                                   // 0x04
    Vector3f scale;                                      // 0x10
    Vector3f direction;                                  // 0x1c
    Vector3f up;                                         // 0x28
    void *frame;                                         // 0x34
    char pad38[0x40 - 0x38];
    FrameNames names;                                    // 0x40
    char pad48[0x54 - 0x48];
    FrameParts parts;                                    // 0x54
    void *Underlying();                                  // 0x4276b0
};

void *FrameProp::Show(void *parent)
{
    ((FrameShow *)this)->Show(parent);
    AddActive(this);
    for (int i = 0; i < names.count; i++) {
        names.Get(i).c_str();
        FrameParts *pp = PARTS;
        void *attached = pp->Get(i)->Attach(Underlying());
        pp->Get(i)->Place(attached, scale);
    }
    return frame;
}
