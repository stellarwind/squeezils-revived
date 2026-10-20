// SqueezilSoul::SpitStanding (0x455f10): self-contained.
#include "compat41.h"

class String {
public:
    String(const char *s);                  // 0x438380
    ~String();                              // 0x4383d0
    char *rep;
};

struct Vector3f {
    float x, y, z;
    Vector3f(const Vector3f &o);            // 0x4183c0
    ~Vector3f();                            // 0x401660
};

struct Matrix {
    float m[4][4];
    Vector3f *Position();                   // 0x41faf0
};

void PlayEffect(String name, Vector3f at); // 0x44a250

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix *GetMatrix();            // slot 3, +0xc
    virtual void b4(); virtual void b5(); virtual void b6(); virtual void b7(); virtual void b8(); virtual void b9();
    virtual void b10(); virtual void b11(); virtual void b12(); virtual void b13(); virtual void b14();
    virtual void b15(); virtual void b16(); virtual void b17(); virtual void b18(); virtual void b19();
    virtual int  GetMode();                 // slot 20, +0x50
    virtual void SetMode(int clip, int blend, int flag);   // slot 21, +0x54
    virtual int  ClipEnded();               // slot 22, +0x58
};

struct GroundMotion {
    float acceleration, deceleration, speed;
    float turnAccel, turnDecel, turnRate;
};

struct GroundControl {
    float forward;
    float turn;
    GroundMotion ground;                    // 0x08
};

struct SqueezilPhysics {
    char  pad[0x24];
    GroundMotion walk;                      // 0x24
    char  pad3c[0x278 - 0x3c];
    float spitVelocity;                     // 0x278
};

enum { SS_STAND = 0, SS_NONE = 24 };
enum { MODE_SPIT_2 = 0x21, MODE_SPIT_3 = 0x22 };

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22();
    virtual int  CanEnter(int state);       // slot 23, +0x5c
    virtual void SetState(int state, int blend);   // slot 24, +0x60
    virtual void s25(); virtual void s26(); virtual void s27();
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31();
    virtual void s32(); virtual void s33(); virtual void s34(); virtual void s35();
    virtual void SpitNut(float speed);      // slot 36, +0x90
    virtual void s37(); virtual void s38(); virtual void s39();
    virtual void s40(); virtual void s41(); virtual void s42();
    virtual void GroundMove(GroundControl *c, int next);   // slot 43, +0xac
    char pad04[0xc - 4];
    int  spitPhase;                         // 0xc
    char pad10[0x48 - 0x10];
    SqueezilPhysics *physics;               // 0x48
    char pad4c[0x54 - 0x4c];
    float spitStrength;                     // 0x54

    void SpitStanding(GroundControl *c, int next);
};

// @00455f10 Spitting while standing: the mouth opens (spit piece 1, then 2) and waits for the spit to be let go (spitStrength
// set), then piece 3 and the nut (SpitVelocity times the strength, at least 0.2). The squeezil may walk meanwhile.
void SqueezilSoul::SpitStanding(GroundControl *c, int next)
{
    int mode = Body()->GetMode();
    c->ground = physics->walk;
    GroundMove(c, SS_NONE);
    switch (spitPhase) {
    case 0:
        spitPhase++;
        // fall through
    case 1:
        if (Body()->ClipEnded()) {
            spitPhase++;
            Body()->SetMode(MODE_SPIT_2, 0, 0);
        }
        break;
    case 3:
        if (mode == MODE_SPIT_3 && Body()->ClipEnded()) {
            SetState(next != SS_NONE && CanEnter(next) ? next : SS_STAND, 1);
            spitPhase = 0;
            return;
        }
        break;
    }
    if (spitStrength != -1.0f) {
        Body()->SetMode(MODE_SPIT_3, 1, 0);
        if (spitStrength < 0.2f)
            spitStrength = 0.2f;
        PlayEffect("spitEffect", *Body()->GetMatrix()->Position());
        SpitNut(physics->spitVelocity * spitStrength);
        spitStrength = -1.0f;
        spitPhase = 3;
    }
}
