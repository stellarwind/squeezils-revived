// Spare declarations: the symbol count steers register choice (padscan: 3 takes chase_ctor from 20 to 8).
void PadDecl1();
void PadDecl2();
void PadDecl3();
// ChaseBehavior constructor (SQZ.EXE 0x403b60, vtable 0x49d14c): a TurnTargetActivity and a StopActivity.
#include "Activity/Activity.h"
struct ChaseBehavior : Behavior {
    Activity *current;                                   // 0x0c
    int mode;                                            // 0x10
    TurnTargetActivity *walk;                            // 0x14
    StopActivity *stop;                                  // 0x18
    int pad1c;                                           // 0x1c
    float rangeSq;                                       // 0x20
    int arg;                                             // 0x24
    ChaseBehavior(ABody *b, ASoul *s, float range, int a);   // 0x403b60
    virtual ~ChaseBehavior();                            // 0x403cb0
    virtual int Execute(int dt, int unused);
};
ChaseBehavior::ChaseBehavior(ABody *b, ASoul *s, float range, int a) : Behavior(b, s), mode(0), walk(new TurnTargetActivity(b, s)), stop(new StopActivity(b, s)), pad1c(0)
{
    rangeSq = range * range;
    arg = a;
    current = stop;
}

// ~ChaseBehavior (0x403cb0)
ChaseBehavior::~ChaseBehavior()
{
    delete walk;
    walk = 0;
    delete stop;
    stop = 0;
}
