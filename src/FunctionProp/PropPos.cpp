// CreatePropAtSqueezilPositionFunction::Position (0x46fce0): where the player's squeezil is.
#define V3_COPYCTOR
#include "FunctionProp/PropClasses.h"

struct PosCall { void *Actor(); };                       // 0x420c20
struct PosBody2 { char pad[0x30]; Vector3f position; };
struct PosV2 { virtual void s0(); virtual void s1(); virtual void s2(); virtual PosBody2 *Transform(); };   // slot 3
struct PosActor2 {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16();
    virtual PosV2 *Body();                               // slot 17
};

Vector3f CreatePropAtSqueezilPositionFunction::Position(ScriptCall *args, int extra)
{
    PosBody2 *t = ((PosActor2 *)((PosCall *)args)->Actor())->Body()->Transform();
    return t->position;
}
