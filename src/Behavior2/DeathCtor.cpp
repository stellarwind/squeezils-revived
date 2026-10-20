// DeathBehavior constructor (SQZ.EXE 0x4030f0) and destructor (0x403250).
#include "Activity/Activity.h"
struct DeathBehavior : Behavior {
    Activity *current;                                   // 0x0c
    int mode;                                            // 0x10
    DieActivity *die;                                    // 0x14
    RegenActivity *regen;                                // 0x18
    float delay;                                         // 0x1c
    int pad20;                                           // 0x20
    DeathBehavior(ABody *b, ASoul *s, float d);          // 0x4030f0
    virtual ~DeathBehavior();                            // 0x403250
    virtual int Execute(int dt, int unused);
};
DeathBehavior::DeathBehavior(ABody *b, ASoul *s, float d) : Behavior(b, s), mode(0), die(new DieActivity(b, s)), regen(new RegenActivity(b, s)), pad20(0)
{
    delay = d;
}
DeathBehavior::~DeathBehavior()
{
    delete die;
    die = 0;
    delete regen;
    regen = 0;
}
