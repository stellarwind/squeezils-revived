// FrameProp::Clone (virtual slot 1, SQZ.EXE 0x426fe0): a new instance of this prop with the same pose, names, cycle and messages.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"

struct FrameOwner;
struct NameObject { int a, b; };
struct MessageObject { int a, b; };
struct FrameProp {
    virtual ~FrameProp();
    virtual FrameProp *Clone(FrameOwner *owner);         // slot 1  0x426fe0
    Vector3f position;                                   // 0x04
    Vector3f scale;                                      // 0x10
    Vector3f direction;                                  // 0x1c
    Vector3f up;                                         // 0x28
    void *frame;                                         // 0x34
    FrameOwner *owner;                                   // 0x38
    int isInstance;                                      // 0x3c
    NameObject names;                                    // 0x40
    char pad48[0x64 - 0x48];
    MessageObject messages;                              // 0x64
    char pad6c[0x98 - 0x6c];
    void *cycle;                                         // 0x98
    char pad9c[0xac - 0x9c];                             // actor index, follow mode, start time, hide-at-end (not touched here)
    FrameProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up, NameObject *names, void *cycle, MessageObject *messages);   // 0x426c70
};

FrameProp *FrameProp::Clone(FrameOwner *newOwner)
{
    FrameProp *f = new FrameProp(position, scale, direction, up, &names, cycle, &messages);
    f->owner = newOwner;
    f->isInstance = 1;
    return f;
}
