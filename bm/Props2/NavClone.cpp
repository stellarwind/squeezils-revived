// NavProp::Clone (virtual slot 1, SQZ.EXE 0x4298f0): a new nav prop with the same pose, names, cycle, messages and nav data.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"

struct NavOwner;
struct NavBlock { int a, b; };
struct NavProp {
    virtual ~NavProp();
    virtual NavProp *Clone(NavOwner *owner);             // slot 1  0x4298f0
    Vector3f position;                                   // 0x04
    Vector3f scale;                                      // 0x10
    Vector3f direction;                                  // 0x1c
    Vector3f up;                                         // 0x28
    void *frame;                                         // 0x34
    NavOwner *owner;                                     // 0x38
    int isInstance;                                      // 0x3c
    NavBlock names;                                      // 0x40
    char pad48[0x64 - 0x48];
    NavBlock messages;                                   // 0x64
    char pad6c[0x98 - 0x6c];
    void *cycle;                                         // 0x98
    char pad9c[0xa4 - 0x9c];
    int actor;                                           // 0xa4
    char pada8[0xac - 0xa8];
    NavBlock start;                                      // 0xac
    char padb4[0xe8 - 0xb4];
    int e8;                                              // 0xe8
    char padec[0xf0 - 0xec];
    int f0;                                              // 0xf0
    int surface;                                         // 0xf4
    int instance;                                        // 0xf8
    NavBlock nav;                                        // 0xfc
    char pad104[0x12c - 0x104];
    int sizeSquared;                                     // 0x12c
    NavProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up, NavBlock *names, void *cycle, NavBlock *messages,
            NavBlock *start, int f0, int e8, int actor, int surface, NavBlock *nav);   // 0x4296d0
};

NavProp *NavProp::Clone(NavOwner *newOwner)
{
    NavProp *n = new NavProp(position, scale, direction, up, &names, cycle, &messages, &start, f0, e8, actor, surface, &nav);
    n->owner = newOwner;
    n->sizeSquared = sizeSquared;
    n->isInstance = 1;
    return n;
}
