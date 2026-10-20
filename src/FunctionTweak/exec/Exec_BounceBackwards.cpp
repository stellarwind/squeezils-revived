// BounceBackwardsFunction::Execute (0x430f20): turn the creature round and add to its speed (only on the ground).
#define V3_COPYCTOR
#include "FunctionTweak/BodyParams.h"

struct MotionBody {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual Vector3f *Direction();                       // slot 4 (0x10)
    virtual float Speed();                               // slot 5 (0x14)
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void SetMotion(Vector3f *dir, float speed);  // slot 10 (0x28)
    virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual int OnGround();                                // slot 15 (0x3c)
};
struct MotionActor {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16();
    virtual MotionBody *Body();                          // slot 17 (0x44)
};

int BounceBackwardsFunction::Execute(ScriptCall *a, int extra)
{
    if (!((MotionActor *)((CallActor *)a)->Actor())->Body()->OnGround()) {
        DebugLog("WARNING: BounceBackwards on a creature that is not moving.\n");
        return 1;
    }
    MotionBody *body = ((MotionActor *)((CallActor *)a)->Actor())->Body();
    Vector3f dir = *body->Direction();
    dir = Vector3f(0, 0, 0) - dir;
    body->SetMotion(&dir, ARGF(0, extra) + body->Speed());
    return 1;
}
