// 5 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
// WanderBehavior constructor (SQZ.EXE 0x4023b0, ret 8): a move, a turn-target and a stop activity; the first two are then set to walk forward.
#include "Activity/Activity.h"
struct WanderBehaviorC : Behavior {
    Activity *current;                                   // 0x0c
    int pause, mode;                                     // 0x10, 0x14
    MoveActivity *idle;                                  // 0x18
    TurnTargetActivity *walk;                            // 0x1c
    StopActivity *spare;                                 // 0x20
    WanderBehaviorC(ABody *b, ASoul *s);
    virtual ~WanderBehaviorC();
    virtual int Execute(int dt, int unused);
};
WanderBehaviorC::WanderBehaviorC(ABody *b, ASoul *s) : Behavior(b, s), pause(50), mode(0), idle(new MoveActivity(b, s)), walk(new TurnTargetActivity(b, s)), spare(new StopActivity(b, s))
{
    idle->direction = 1;
    idle->turn = 0;
    idle->gait = 3;
    walk->direction = 1;
    walk->turn = 0;
    walk->gait = 0;
    current = spare;
}
WanderBehaviorC::~WanderBehaviorC() {}
int WanderBehaviorC::Execute(int dt, int unused) { return 0; }
