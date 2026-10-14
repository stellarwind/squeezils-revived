// SqueezilPlayerSoul::ReadGroundInput (slot 39, SQZ.EXE 0x41f1c0, ret 8): what the player wishes on the ground. Draft.
#include <math.h>
struct Motion24 { int w[6]; };                           // the first 6 dwords of a GroundMotion
struct GroundControl {
    float forward;                                       // 0x00
    float turn;                                          // 0x04
    Motion24 ground;                                     // 0x08
};
struct Application {
    char pad[0x64];
    int runHeld;                                         // 0x64
    int attackHeld;                                      // 0x68
    float TurnAxis();                                    // 0x43bd20
    float ForwardAxis();                                 // 0x43bc70
    float ForwardTap();                                  // 0x43bc10
    float TurnTap();                                     // 0x43bc40
};
Application *GetApplication();                           // 0x4206b0 (cdecl)
struct Physics {
    char pad[0x24];
    Motion24 walk;                                       // 0x24
    char pad2[0x28c - 0x3c];
    float fwdTapFactor;                                  // 0x28c
    float rotTapFactor;                                  // 0x290
};
struct Vec { float x, y, z;
    float Along(float *matrix) { return z * matrix[10] + y * matrix[9] + matrix[8] * x; }
};
struct Body {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual float *GetMatrix();                          // slot 3 (+0xc)
    virtual Vec *GetFacing();                            // slot 4 (+0x10)
    virtual float GetSpeed();                            // slot 5 (+0x14)
};
#define D1(n) virtual void n();
struct SqueezilPlayerSoul {
    D1(v0) D1(v1) D1(v2) D1(v3) D1(v4) D1(v5) D1(v6) D1(v7) D1(v8) D1(v9) D1(v10) D1(v11) D1(v12) D1(v13) D1(v14) D1(v15) D1(v16)
    virtual Body *GetBody();                             // slot 17 (+0x44)
    D1(v18) D1(v19) D1(v20) D1(v21) D1(v22) D1(v23) D1(v24)
    virtual int GetState();                              // slot 25 (+0x64)
    D1(v26) D1(v27) D1(v28) D1(v29) D1(v30) D1(v31) D1(v32) D1(v33) D1(v34) D1(v35) D1(v36) D1(v37) D1(v38) D1(v39) D1(v40)
    virtual void GetGroundMotion(int state, GroundControl *c);   // slot 41 (+0xa4)
    char pad[0x44 - 4];
    int frozen;                                          // 0x44
    Physics *physics;                                    // 0x48
    char pad2[0x84 - 0x4c];
    int attackHeld;                                      // 0x84
    void ReadGroundInput(GroundControl *c, int *next);
};

inline float Along(Vec *direction, float *matrix) { return direction->z * matrix[10] + direction->y * matrix[9] + matrix[8] * direction->x; }

void SqueezilPlayerSoul::ReadGroundInput(GroundControl *c, int *next)
{
    struct Input { float forward, speed, turn, forwardTap, turnTap; int run; } input;
    attackHeld = GetApplication()->attackHeld;
    input.run = GetApplication()->runHeld;
    input.turn = GetApplication()->TurnAxis();
    input.forward = GetApplication()->ForwardAxis();
    input.speed = GetBody()->GetSpeed();
    int current = GetState();
    if (frozen) {
        c->ground = physics->walk;
        c->forward = 0.0f;
        c->turn = 0.0f;
        *next = 0;
        return;
    }
    input.forwardTap = GetApplication()->ForwardTap();
    input.turnTap = GetApplication()->TurnTap();
    if (input.forwardTap != 0.0f && fabs(input.forward) <= 0.01)
        input.forward = physics->fwdTapFactor * input.forwardTap;
    if (input.turnTap != 0.0f && fabs(input.turn) <= 0.01)
        input.turn = physics->rotTapFactor * input.turnTap;

    int wish;
    if (fabs(input.forward) > 0.01) {
        if (attackHeld)
            wish = 7;
        else if (input.forward < 0.0f && input.speed > 0.01f && GetBody()->GetFacing()->Along(GetBody()->GetMatrix()) > 0.0f)
            wish = 5;
        else
            wish = input.run ? 4 : 3;
        c->forward = input.forward;
    } else {
        wish = 0;
        c->forward = 0.0f;
    }
    if (wish != current) *next = wish;
    else *next = 0x18;
    GetGroundMotion(current, c);
    c->turn = (fabs(input.turn) > 0.01 && current != 6) ? input.turn : 0.0f;
}
