// CycleProp::Show (virtual slot 3, SQZ.EXE 0x428d80): show as a frame prop, join the active list, attach the current cycle part.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "String/String.h"

struct CyclePart {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void *Attach(void *object);                  // slot 4 (+0x10)
    virtual void v5();
    virtual void Place(void *attached, Vector3f scale);  // slot 6 (+0x18)
};
struct CycleNames {                                      // embedded at +0x40
    virtual String Get(int i);                           // slot 0
    int count;
};
struct CycleParts {                                      // embedded at +0x54
    virtual CyclePart *Get(int i);                       // slot 0
    int count;
};
struct FrameShow { void *Show(void *parent); };          // 0x427380
void AddActive(void *prop);                              // 0x42d180 (cdecl)

struct CycleProp {
    virtual ~CycleProp();
    virtual CycleProp *Clone(void *owner);               // slot 1
    virtual void v2();
    virtual void *Show(void *parent);                    // slot 3  0x428d80
    Vector3f position;                                   // 0x04
    Vector3f scale;                                      // 0x10
    Vector3f direction;                                  // 0x1c
    Vector3f up;                                         // 0x28
    void *frame;                                         // 0x34
    char pad38[0x40 - 0x38];
    CycleNames names;                                    // 0x40
    char pad48[0x54 - 0x48];
    CycleParts parts;                                    // 0x54
    char pad5c[0xdc - 0x5c];
    int current;                                         // 0xdc
    void *Underlying();                                  // 0x4276b0
};

void *CycleProp::Show(void *parent)
{
    ((FrameShow *)this)->Show(parent);
    AddActive(this);
    names.Get(current).c_str();
    void *attached = parts.Get(current)->Attach(Underlying());
    parts.Get(current)->Place(attached, scale);
    return frame;
}
