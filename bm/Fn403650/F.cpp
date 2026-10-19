// Behavior constructor with a move, a turn-target and a stop activity (SQZ.EXE 0x403650, ret 8).
#include "Activity/Activity.h"
struct ThreeBehavior : Behavior {
    Activity *current;                                   // 0x0c
    int mode;                                            // 0x10
    MoveActivity *move;                                  // 0x14
    TurnTargetActivity *turn;                            // 0x18
    StopActivity *stop;                                  // 0x1c
    ThreeBehavior(ABody *b, ASoul *s);
    virtual ~ThreeBehavior();
    virtual int Execute(int dt, int unused);
};
ThreeBehavior::ThreeBehavior(ABody *b, ASoul *s) : Behavior(b, s), mode(0), move(new MoveActivity(b, s)), turn(new TurnTargetActivity(b, s)), stop(new StopActivity(b, s))
{
    current = stop;
}
ThreeBehavior::~ThreeBehavior() {}
int ThreeBehavior::Execute(int dt, int unused) { return 0; }
