// FleeBehavior::Execute (0x404120): run away from a creature: pick a point opposite to it, walk there, watch its speed.
#define V3_COPYCTOR
#include "Behavior2/Activity.h"
#include "Vector3f/Vector3f.h"
#include <math.h>

struct FleeSub {                                         // walk / idle sub-behaviours
    virtual int Run(int a);                              // slot 0
    virtual int Execute(int a);                          // slot 1
    char pad[0x2ac - 4];
    int heading;                                         // 0x2ac
    int pad2b0;
    unsigned flags;                                      // 0x2b4
    int mode;                                            // 0x2b8
    Vector3f dest;                                       // 0x2bc
};
struct FleeCreature {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual float Speed();                               // slot 5 (+0x14)
};
struct FleeXform { char pad[0x30]; Vector3f pos; };      // 0x30
struct FleeBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual FleeXform *Transform();                      // slot 3 (+0xc)
};
struct FleeRecord {
    int id;                                              // 0x00
    Vector3f pos;                                        // 0x04
    float weight;                                        // 0x10
    int pad14;
};
struct FleeSoul {
    char pad[0xa0 - 4];
    int index;                                           // 0xa0
    FleeRecord records[1];                               // 0xa4
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual int State();                                 // slot 25 (+0x64)
};
struct FleeSim { FleeCreature *GetCreature(int id); };   // 0x414050
FleeSim *GetSimulation();                                // 0x420690 (cdecl)

struct FleeBehavior {
    virtual void s0();
    virtual int Execute(int a, int b);                   // slot 1
    FleeBody *body;                                      // 0x04
    FleeSoul *soul;                                      // 0x08
    FleeSub *current;                                    // 0x0c
    int mode;                                            // 0x10
    FleeSub *idle;                                       // 0x14
    FleeSub *walk;                                       // 0x18
    int timer;                                           // 0x1c
};

int FleeBehavior::Execute(int a, int b)
{
    FleeRecord *rec = &soul->records[soul->index];
    if (--timer <= 0) {
        switch (mode) {
        case 0:
            if (soul->State() != 8 && soul->State() != 6) {
                current = idle;
                idle->mode = 3;
                idle->heading = -1;
                timer = 10;
                mode++;
            }
            break;
        case 1: {
            Vector3f dest, p, d;
            current = walk;
            walk->mode = 3;
            FleeXform *t = body->Transform();
            p = t->pos;
            d = rec->pos - p;
            dest = p + d * -10.0f;
            FleeSub *w = walk;
            w->dest = dest;
            mode++;
            break;
        }
        case 2:
            if (fabs(GetSimulation()->GetCreature(rec->id)->Speed()) > 0.01)
                mode--;
            else if (!(walk->flags & 0x7fffffff))
                mode++;
            break;
        case 3:
            current = idle;
            idle->heading = 1;
            idle->mode = 3;
            mode++;
            break;
        }
    }
    return current->Execute(a);
}
