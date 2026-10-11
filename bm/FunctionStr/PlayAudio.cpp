// PlayAudioEffectFunction::Execute (0x43a130): play the effect at the speaking creature's position.
#define V3_COPYCTOR_OUTOFLINE
#include "FunctionStr/Str.h"
#include "Vector3f/Vector3f.h"

struct AudioCall {                       // ScriptCall seen through its accessors
    void *Actor();                       // 0x420c20
    void *Other();                       // 0x420c30
};
struct PosBody { char pad[0x30]; float px, py, pz; };   // +0x30
struct PosV {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual PosBody *Transform();        // slot 3 (0xc)
};
struct PosActor {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16();
    virtual PosV *Body();                // slot 17 (0x44)
};
void PlayEffectAt(String name, Vector3f position);       // 0x44a250 (cdecl)

int PlayAudioEffectFunction::Execute(ScriptCall *args, int extra)
{
    Vector3f pos;
    pos.x = 0; pos.y = 0; pos.z = 0;
    AudioCall *c = (AudioCall *)args;
    if (c->Actor())
    {
        PosBody *t = ((PosActor *)c->Actor())->Body()->Transform();
        pos.x = t->px; pos.y = t->py; pos.z = t->pz;
    } else if (c->Other()) {
        PosBody *t = ((PosActor *)c->Actor())->Body()->Transform();
        pos.x = t->px; pos.y = t->py; pos.z = t->pz;
    }
    PlayEffectAt(text, pos);
    return 1;
}
