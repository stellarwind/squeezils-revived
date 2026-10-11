// SimpleFlyBehavior::Execute (0x403580): on the first call point the fly target at the body's position (height clamped), then run the sub-behaviour.
#define V3_COPYCTOR
#include "Behavior2/Activity.h"
#include "Vector3f/Vector3f.h"

struct FlyXform { char pad[0x30]; Vector3f pos; };     // 0x30
struct FlyBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual FlyXform *Transform();                       // slot 3 (+0xc)
};
struct FlyTarget {
    char pad[0x2b4 - 4];
    Vector3f pos;                                        // 0x2b4 (the vptr takes 4)
    virtual void v0();
};
struct FlySub {
    virtual int Run(int a);                              // slot 0
};
struct SimpleFlyBehavior {
    virtual void s0();
    virtual int Execute(int a, int b);                   // slot 1
    FlyBody *body;                                       // 0x04
    void *soul;                                          // 0x08
    FlySub *current;                                     // 0x0c
    int started;                                         // 0x10
    FlyTarget *target;                                   // 0x14
};

int SimpleFlyBehavior::Execute(int a, int b)
{
    Vector3f v;
    if (!started) {
        current = (FlySub *)target;
        FlyXform *t = body->Transform();
        v = t->pos;
        if (v.y > 10.0f)
            v.y = 10.0f;
        else
            v.y = 1.0f;
        FlyTarget *tg = target;
        tg->pos.x = v.x;
        tg->pos.y = v.y;
        tg->pos.z = v.z;
        started++;
    }
    return current->Run(b);
}
