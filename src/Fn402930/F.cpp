// PaceBehavior constructor (SQZ.EXE 0x402930, ret 0x14): the three activities, then the pace speed from the soul's walk speed.
#include "Activity/Activity.h"
struct PacePhysics {                                     // a copy of the soul's SqueezilPhysics (0x2a0 bytes)
    float head[0xb];
    float walkSpeed;                                     // 0x2c
    char rest[0x2a0 - 0x30];
    PacePhysics(const APhysics &o);                      // 0x405040
    ~PacePhysics();                                      // 0x405260
};
struct PaceBehaviorC : Behavior {
    Activity *current;                                   // 0x0c
    int a10, a14;                                        // 0x10
    MoveActivity *idle;                                  // 0x18
    TurnTargetActivity *walk;                            // 0x1c
    StopActivity *stop;                                  // 0x20
    int mode;                                            // 0x24
    float speedRatio;                                    // 0x28
    int count;                                           // 0x2c
    PaceBehaviorC(ABody *b, ASoul *s, int m, float range, int c);
    virtual ~PaceBehaviorC();
    virtual int Execute(int dt, int unused);
};
PaceBehaviorC::PaceBehaviorC(ABody *b, ASoul *s, int m, float range, int c) : Behavior(b, s), a10(0), a14(0), idle(new MoveActivity(b, s)), walk(new TurnTargetActivity(b, s)), stop(new StopActivity(b, s))
{
    count = c;
    mode = m;
    PacePhysics p(*s->physics);
    speedRatio = range / p.walkSpeed;
    idle->direction = 1;
    idle->turn = 0;
    idle->gait = 3;
    walk->direction = 1;
    walk->turn = 0;
    walk->gait = 0;
    current = stop;
}
PaceBehaviorC::~PaceBehaviorC() {}
int PaceBehaviorC::Execute(int dt, int unused) { return 0; }
