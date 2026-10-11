// 28 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
void PadDecl18();
void PadDecl19();
void PadDecl20();
void PadDecl21();
void PadDecl22();
void PadDecl23();
void PadDecl24();
void PadDecl25();
void PadDecl26();
void PadDecl27();
void PadDecl28();
// ChaseBehavior::Execute (0x403930): aim the chase target at where the chased creature will be, step the approach state machine.
#define V3_COPYCTOR
#include "Behavior2/Activity.h"
#include "Vector3f/Vector3f.h"
#include <math.h>
#pragma intrinsic(sqrt)

struct ChaseTarget {                                     // the sub-behaviour that walks to a point
    virtual int Run(int a);                              // slot 0
    virtual int Execute(int a);                          // slot 1
    char pad[0x2b4 - 4];
    unsigned flags;                                      // 0x2b4
    int mode;                                            // 0x2b8
    Vector3f dest;                                       // 0x2bc
    float Distance();                                    // 0x4016e0
};
struct ChaseCreature {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual Vector3f *Velocity();                        // slot 4 (+0x10)
    virtual float Speed();                               // slot 5 (+0x14)
};
struct ChaseBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual float Speed();                               // slot 5 (+0x14)
};
struct ChaseRecord {                                     // 24 bytes, one per tracked creature
    int id;                                              // 0x00
    Vector3f pos;                                        // 0x04
    float weight;                                        // 0x10
    int pad14;
};
struct ChaseSoul {
    char pad[0xa0 - 4];
    int index;                                           // 0xa0 (the vptr takes 4)
    ChaseRecord records[1];                              // 0xa4
    virtual void v0();
};
struct ChaseSim { ChaseCreature *GetCreature(int id); }; // 0x414050
ChaseSim *GetSimulation();                               // 0x420690 (cdecl)

struct ChaseBehavior {
    virtual void s0();
    virtual int Execute(int a, int b);                   // slot 1
    ChaseBody *body;                                     // 0x04
    ChaseSoul *soul;                                     // 0x08
    ChaseTarget *current;                                // 0x0c
    int mode;                                            // 0x10
    int pad14;
    ChaseTarget *target;                                 // 0x18
};

int ChaseBehavior::Execute(int a, int b)
{
    Vector3f vel;
    Vector3f aim;
    ChaseRecord *rec = &soul->records[soul->index];
    ChaseCreature *who = GetSimulation()->GetCreature(rec->id);
    vel = *who->Velocity() * who->Speed();
    float speed = body->Speed();
    float ratio = (float)sqrt(rec->weight) / speed;
    if (ratio > 2.0f)
        ratio = 2.0f;
    aim = rec->pos + vel * ratio;
    ChaseTarget *tg = target;
    tg->dest.x = aim.x;
    tg->dest.y = aim.y;
    tg->dest.z = aim.z;
    switch (mode) {
    case 0:
        current = target;
        target->mode = 3;
        mode++;
        break;
    case 1:
        if (!(target->flags & 0x7fffffff)) {
            target->mode = 4;
            mode++;
        }
        break;
    case 2:
        if (target->Distance() > 50.0f) {
            target->mode = 3;
            mode--;
        }
        break;
    }
    return current->Execute(a);
}
