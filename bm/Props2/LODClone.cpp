// LODProp::Clone (virtual slot 1, SQZ.EXE 0x4282d0): a new instance of this prop with the same pose, names, cycle and messages.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"

struct LODOwner;
struct LODNames { int a, b; };
struct LODMessages { int a, b; };
struct LODTable { int a, b; };
struct LODProp {
    virtual ~LODProp();
    virtual LODProp *Clone(LODOwner *owner);         // slot 1  0x4282d0
    Vector3f position;                                   // 0x04
    Vector3f scale;                                      // 0x10
    Vector3f direction;                                  // 0x1c
    Vector3f up;                                         // 0x28
    void *frame;                                         // 0x34
    LODOwner *owner;                                   // 0x38
    int isInstance;                                      // 0x3c
    LODNames names;                                    // 0x40
    char pad48[0x64 - 0x48];
    LODMessages messages;                              // 0x64
    char pad6c[0x98 - 0x6c];
    void *cycle;                                         // 0x98
    char pad9c[0xb0 - 0x9c];
    LODTable lod;                                        // 0xb0
    char padb8[0xc4 - 0xb8];
    LODProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up, LODNames *names, void *cycle, LODMessages *messages, LODTable *lod);   // 0x428050
};

LODProp *LODProp::Clone(LODOwner *newOwner)
{
    LODProp *f = new LODProp(position, scale, direction, up, &names, cycle, &messages, &lod);
    f->owner = newOwner;
    f->isInstance = 1;
    return f;
}
