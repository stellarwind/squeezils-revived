// CycleProp::Clone (virtual slot 1, SQZ.EXE 0x428bd0): a new cycle prop with the same pose, names, cycle, messages and timing.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"

struct CycleOwner;
struct CycleBlock { int a, b; };
struct CycleProp {
    virtual ~CycleProp();
    virtual CycleProp *Clone(CycleOwner *owner);         // slot 1  0x428bd0
    Vector3f position;                                   // 0x04
    Vector3f scale;                                      // 0x10
    Vector3f direction;                                  // 0x1c
    Vector3f up;                                         // 0x28
    void *frame;                                         // 0x34
    CycleOwner *owner;                                   // 0x38
    int isInstance;                                      // 0x3c
    CycleBlock names;                                    // 0x40
    char pad48[0x64 - 0x48];
    CycleBlock messages;                                 // 0x64
    char pad6c[0x98 - 0x6c];
    void *cycle;                                         // 0x98
    char pad9c[0xa4 - 0x9c];
    int actor;                                           // 0xa4
    char pada8[0xac - 0xa8];
    CycleBlock start;                                    // 0xac
    char padb4[0xdc - 0xb4];
    int dc;                                              // 0xdc
    char pade0[0xe8 - 0xe0];
    int e8;                                              // 0xe8
    char padec[0xf0 - 0xec];
    int f0;                                              // 0xf0
    CycleProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up, CycleBlock *names, void *cycle, CycleBlock *messages,
              CycleBlock *start, int f0, int e8, int actor);   // 0x4288b0
};

CycleProp *CycleProp::Clone(CycleOwner *newOwner)
{
    CycleProp *c = new CycleProp(position, scale, direction, up, &names, cycle, &messages, &start, f0, e8, actor);
    c->dc = dc;
    c->owner = newOwner;
    c->isInstance = 1;
    return c;
}
