// Activity::Activity (0x401000) and RefreshPhysics (0x401270), both exact. The ctor assigns the soul's physics block with the compiler's memberwise
// operator= (AV3::operator= out of line = 0x401670, the first vector copied by hand in APhysical::operator=); RefreshPhysics has all the
// copies inline, so it is written field by field.
#include "Activity/Activity.h"
Activity::Activity(ABody *b, ASoul *s)
{
    body = b;
    soul = s;
    physics = *s->physics;
}
void Activity::RefreshPhysics()
{
    APhysics *p = soul->physics;
    physics.physical.gravity.x = p->physical.gravity.x;
    physics.physical.gravity.y = p->physical.gravity.y;
    physics.physical.gravity.z = p->physical.gravity.z;
    physics.physical.down.x = p->physical.down.x;
    physics.physical.down.y = p->physical.down.y;
    physics.physical.down.z = p->physical.down.z;
    physics.physical.gravityStrength = p->physical.gravityStrength;
    physics.physical.kineticFriction = p->physical.kineticFriction;
    physics.physical.staticFriction = p->physical.staticFriction;
    physics.walk = p->walk; physics.run = p->run; physics.scamper = p->scamper; physics.skid = p->skid;
    physics.fly = p->fly; physics.dive = p->dive; physics.tumble = p->tumble;
    physics.jump = p->jump; physics.hop = p->hop; physics.skihop = p->skihop;
    physics.attack = p->attack; physics.slip = p->slip;
    physics.t0 = p->t0; physics.t1 = p->t1; physics.t2 = p->t2; physics.t3 = p->t3; physics.t4 = p->t4;
    physics.t5 = p->t5; physics.t6 = p->t6; physics.t7 = p->t7; physics.t8 = p->t8; physics.t9 = p->t9;
    physics.t10 = p->t10; physics.t11 = p->t11; physics.t12 = p->t12; physics.t13 = p->t13;
}
