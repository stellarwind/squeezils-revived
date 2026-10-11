// SqueezilRoboSoul::Sense (0x404720) and TargetGone (0x4047c0).
#include "SqueezilRoboSoul/SqueezilRoboSoul.h"
#include "Simulation/Simulation.h"

Simulation *GetCurrentSimulation();                                      // 0x420690
void *DynamicCast(void *p, int zero, const void *from, const void *to, int ref);   // 0x48f060 (cdecl)
extern const char kSoulType[];                                           // 0x4b93e8
extern const char kSimulationSoulType[];                                 // 0x4b9400

struct KindSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual int Kind();   // slot 8
};

void SqueezilRoboSoul::Sense(const AwarenessRecord *seen)
{
    KindSoul *s = (KindSoul *)DynamicCast(GetCurrentSimulation()->GetSoul(seen->id), 0, kSoulType, kSimulationSoulType, 0);
    int kind = s->Kind();
    if (kind < 3)
        records[kind].Offer(seen);
}

int SqueezilRoboSoul::TargetGone(int kind)
{
    if (kind < 3 && records[kind].id != -1) {
        if (!GetCurrentSimulation()->GetActor(records[kind].id)
            || ((KindSoul *)DynamicCast(GetCurrentSimulation()->GetSoul(records[kind].id), 0, kSoulType, kSimulationSoulType, 0))->Kind() == 6)
            return 1;
    }
    return 0;
}

struct BodyV {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual int IsOnGround();   // slot 15
};
struct SoulV {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16();
    virtual BodyV *Body();      // slot 17
};

int SqueezilRoboSoul::SenseState(int kind)
{
    AwarenessRecord *record = &records[kind];
    int *last = &lastSeen[kind];
    int onGround = ((SoulV *)this)->Body()->IsOnGround();
    int action;
    if (IsNear(kind)) {
        int gone = TargetGone(kind);
        if (*last == record->id && !retarget) {
            if (gone) {
                action = 1;
                record->id = -1;
            } else {
                action = 2;
            }
        } else if (gone) {
            record->id = -1;
            action = 1 + (*last == -1);
            *last = record->id;
        } else {
            action = 0;
            *last = record->id;
        }
    } else {
        action = 1 + (*last == -1);
    }
    switch (action) {
    case 0:
        senseKind = kind;
        switch (kind) {
        case 0: return onGround ? 5 : 6;
        case 1: return onGround ? 7 : 8;
        case 2: return onGround ? 9 : 10;
        }
        return onGround;
    case 1:
        ClearSeen();
        senseKind = 999;
        return onGround ? 0xb : 0xc;
    case 2:
        senseKind = 999;
        return 0x13;
    }
    return 0x13;
}
