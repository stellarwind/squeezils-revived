// SqueezilSoul::Update (0x450e90): self-contained (the per-state handlers are virtual).
#include "compat41.h"

struct Matrix {
    float m[4][4];
};

struct Matrix16 {                           // a body matrix held by value (64 bytes)
    float v[16];
    Matrix16(const Matrix &o)
    {
        v[0] = o.m[0][0];
        v[1] = o.m[0][1];
        v[2] = o.m[0][2];
        v[3] = o.m[0][3];
        v[4] = o.m[1][0];
        v[5] = o.m[1][1];
        v[6] = o.m[1][2];
        v[7] = o.m[1][3];
        v[8] = o.m[2][0];
        v[9] = o.m[2][1];
        v[10] = o.m[2][2];
        v[11] = o.m[2][3];
        v[12] = o.m[3][0];
        v[13] = o.m[3][1];
        v[14] = o.m[3][2];
        v[15] = o.m[3][3];
    }
    ~Matrix16();                            // 0x402310
    void Orthonormalize();                  // 0x47e350
};

struct Contact;

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix *GetMatrix();            // slot 3, +0xc
    virtual void b4(); virtual void b5(); virtual void b6();
    virtual Contact *GetContact();          // slot 7, +0x1c
    virtual void b8();
    virtual void SetMatrix(Matrix16 *m);    // slot 9, +0x24
    virtual void b10(); virtual void b11(); virtual void b12(); virtual void b13(); virtual void b14();
    virtual int  IsOnGround();              // slot 15, +0x3c
    virtual void b16(); virtual void b17(); virtual void b18(); virtual void b19();
    virtual int  GetMode();                 // slot 20, +0x50
    virtual void SetMode(int clip, int blend, int flag);   // slot 21, +0x54
    virtual int  ClipEnded();               // slot 22, +0x58
};

struct NavSurfaces {
    int IsGone(Contact *c);                 // 0x410890
};

struct Place {
    NavSurfaces *Surfaces();                // 0x439b60
};

struct GroundControl;
struct AirControl;
extern GroundControl gGroundControl;        // 0x4c7ad8
extern AirControl gAirControl;              // 0x4c7b10

enum {
    SS_STAND, SS_LIE_DOWN, SS_GET_UP, SS_WALK, SS_RUN, SS_SKID, SS_SLIP, SS_ATTACK, SS_STUMBLE, SS_SCAMPER,
    SS_SCAMPER_STOP, SS_TUMBLE, SS_JUMP, SS_HOP_START, SS_HOP, SS_SKIHOP, SS_SPIT, SS_FLY, SS_DIVE, SS_LAND,
    SS_DYING_IN_AIR, SS_DYING, SS_DEAD, SS_UNUSED23, SS_NONE
};
enum { MODE_STOP = 0, MODE_SCAMPER_STOP = 1 };

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void SetState(int state, int blend);   // slot 24, +0x60
    virtual int  GetState();                // slot 25, +0x64
    virtual void s26(); virtual void s27();
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31();
    virtual void s32(); virtual void s33(); virtual void s34();
    virtual void StartTumble();             // slot 35, +0x8c
    virtual void s36(); virtual void s37();
    virtual void AdjustNextState(int *next);                        // slot 38, +0x98
    virtual void ReadGroundInput(GroundControl *c, int *next);      // slot 39, +0x9c
    virtual void ReadAirInput(AirControl *c, int *next);            // slot 40, +0xa0
    virtual void s41(); virtual void s42();
    virtual void GroundMove(GroundControl *c, int next);            // slot 43, +0xac
    virtual void Slip(GroundControl *c, int next);                  // slot 44, +0xb0
    virtual void Fly(AirControl *c, int next);                      // slot 45, +0xb4
    virtual void HopStart(GroundControl *c, int next);              // slot 46, +0xb8
    virtual void SpitStanding(GroundControl *c, int next);          // slot 47, +0xbc
    virtual void Attack(GroundControl *c, int next);                // slot 48, +0xc0
    virtual void Stumble(GroundControl *c, int next);               // slot 49, +0xc4
    virtual void LieDown(GroundControl *c, int next);               // slot 50, +0xc8
    virtual void GetUp(GroundControl *c, int next);                 // slot 51, +0xcc
    virtual void Land(GroundControl *c, int next);                  // slot 52, +0xd0
    virtual void Dying(GroundControl *c, int next);                 // slot 53, +0xd4
    virtual void Skid(GroundControl *c, int next);                  // slot 54, +0xd8
    virtual void JumpFlight(AirControl *c, int next);               // slot 55, +0xdc
    virtual void HopFlight(AirControl *c, int next);                // slot 56, +0xe0
    virtual void SkiHopFlight(AirControl *c, int next);             // slot 57, +0xe4
    virtual void Tumble(AirControl *c, int next);                   // slot 58, +0xe8
    virtual void DyingInAir(AirControl *c, int next);               // slot 59, +0xec
    char pad04[0x70 - 4];
    Place *place;                           // 0x70

    void Update();
};

// @00450e90 A squeezil whose surface copy went away starts tumbling. Then the subclass says what it wants, and the current
// state's handler acts on it; at the end the body's matrix is squared up again.
void SqueezilSoul::Update()
{
    int onGround = Body()->IsOnGround();
    if (onGround && place->Surfaces()->IsGone(Body()->GetContact())) {
        StartTumble();
        onGround = Body()->IsOnGround();
    }
    int current = GetState();
    int next = SS_NONE;
    if (onGround) {
        ReadGroundInput(&gGroundControl, &next);
        AdjustNextState(&next);
        switch (current) {
        case SS_STAND:
        case SS_SCAMPER_STOP:
            if (Body()->GetMode() == MODE_STOP || Body()->GetMode() == MODE_SCAMPER_STOP)
                goto groundMove;
            if (Body()->ClipEnded())
                Body()->SetMode(MODE_STOP, 0, 0);
            break;
        case SS_LIE_DOWN:   LieDown(&gGroundControl, next); break;
        case SS_GET_UP:     GetUp(&gGroundControl, next); break;
        case SS_SKID:       Skid(&gGroundControl, next); break;
        case SS_SLIP:       Slip(&gGroundControl, next); break;
        case SS_ATTACK:     Attack(&gGroundControl, next); break;
        case SS_STUMBLE:    Stumble(&gGroundControl, next); break;
        case SS_HOP_START:  HopStart(&gGroundControl, next); break;
        case SS_SPIT:       SpitStanding(&gGroundControl, next); break;
        case SS_LAND:       Land(&gGroundControl, next); break;
        case SS_DYING:      Dying(&gGroundControl, next); break;
        case SS_WALK:
        case SS_RUN:
        case SS_SCAMPER:
        groundMove:
            GroundMove(&gGroundControl, next);
            break;
        case SS_DEAD:       SetState(next, 0); break;
        }
    } else {
        ReadAirInput(&gAirControl, &next);
        switch (current) {
        case SS_TUMBLE:         Tumble(&gAirControl, next); break;
        case SS_JUMP:           JumpFlight(&gAirControl, next); break;
        case SS_HOP:            HopFlight(&gAirControl, next); break;
        case SS_SKIHOP:         SkiHopFlight(&gAirControl, next); break;
        case SS_FLY:
        case SS_DIVE:
            Fly(&gAirControl, next);
            break;
        case SS_DYING_IN_AIR:   DyingInAir(&gAirControl, next); break;
        case SS_DEAD:           SetState(next, 0); break;
        }
    }
    Matrix16 m = *Body()->GetMatrix();
    m.Orthonormalize();
    Body()->SetMatrix(&m);
}
