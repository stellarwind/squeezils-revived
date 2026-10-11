// SetMotionSpeedFunction::Execute (0x4306f0): keep the direction, set the speed.
#define V3_COPYCTOR
#include "FunctionTweak/BodyParams.h"

struct MotionBody {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual Vector3f *Direction();                       // slot 4 (0x10)
    virtual float Speed();                               // slot 5 (0x14)
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void SetMotion(Vector3f *dir, float speed);  // slot 10 (0x28)
};
struct MotionActor {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16();
    virtual MotionBody *Body();                          // slot 17 (0x44)
};

int SetMotionSpeedFunction::Execute(ScriptCall *a, int extra)
{
    Vector3f dir = *((MotionActor *)((CallActor *)a)->Actor())->Body()->Direction();
    ((MotionActor *)((CallActor *)a)->Actor())->Body()->SetMotion(&dir, ARGF(0, extra));
    return 1;
}
