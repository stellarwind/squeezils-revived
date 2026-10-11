// AccelerateFunction::Execute (0x431080): add to the creature's speed; reverse the direction if that changes the sign.
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

int AccelerateFunction::Execute(ScriptCall *a, int extra)
{
    MotionBody *body = ((MotionActor *)((CallActor *)a)->Actor())->Body();
    float speed = body->Speed();
    float next = ARGF(0, extra) + speed;
    Vector3f dir = *body->Direction();
    if (*(unsigned *)&speed <= 0x80000000 && *(unsigned *)&next > 0x80000000) {
        dir = Vector3f(0, 0, 0) - dir;
        next = -next;
    }
    body->SetMotion(&dir, next);
    return 1;
}
