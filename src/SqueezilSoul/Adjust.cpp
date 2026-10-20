// SqueezilSoul::AdjustNextState (0x450c20): self-contained.
// 1 spare declaration (symbol count, see padscan.sh)
void PadDecl1();
#include <math.h>
#pragma intrinsic(acos)
#include "compat41.h"

struct Matrix {
    float m[4][4];                          // rows: right, up, forward, position
};

struct Vector3f {
    float x, y, z;
    Vector3f(const Matrix *m, int row) { x = m->m[row][0]; y = m->m[row][1]; z = m->m[row][2]; }
    ~Vector3f();                            // 0x401660
    Vector3f operator*(float s) const;      // 0x401b60
};

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix *GetMatrix();            // slot 3, +0xc
};

struct SqueezilPhysics {
    char     pad[0xc];
    Vector3f gravity;                       // 0xc
    char     pad18[0x27c - 0x18];
    float    walkToScamperAngle;            // 0x27c
    float    scamperToWalkAngle;            // 0x280
};

enum { SS_STAND = 0, SS_WALK = 3, SS_SCAMPER = 9, SS_SCAMPER_STOP = 10, SS_NONE = 24 };

const float kDegreesPerRadian = 57.29578f;

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void s24();
    virtual int  GetState();                // slot 25, +0x64
    char pad04[0x48 - 4];
    SqueezilPhysics *physics;               // 0x48

    void AdjustNextState(int *next);
};

// @00450c20 On a slope steeper than WalkToScamperAngle a walk becomes a scamper; below ScamperToWalkAngle a scamper becomes
// a walk again; stopping on a steep slope uses the scamper stop.
void SqueezilSoul::AdjustNextState(int *next)
{
    int current = GetState();
    if (*next == SS_WALK || (*next == SS_NONE && (current == SS_WALK || current == SS_SCAMPER))) {
        Vector3f up = physics->gravity * -1.0f;
        Vector3f bodyUp(Body()->GetMatrix(), 1);
        float slope = acos(bodyUp.x * up.x + bodyUp.y * up.y + bodyUp.z * up.z) * kDegreesPerRadian;
        if (current == SS_WALK || current == SS_STAND) {
            if (physics->walkToScamperAngle < slope)
                *next = SS_SCAMPER;
        } else if (current == SS_SCAMPER) {
            *next = SS_NONE;
            if (physics->scamperToWalkAngle > slope)
                *next = SS_WALK;
        } else if (current == SS_SCAMPER_STOP) {
            if (physics->scamperToWalkAngle > slope)
                *next = SS_WALK;
            else
                *next = SS_SCAMPER;
        }
    } else if (*next == SS_STAND && (current == SS_SCAMPER || current == SS_SCAMPER_STOP)) {
        Vector3f up = physics->gravity * -1.0f;
        Vector3f bodyUp(Body()->GetMatrix(), 1);
        if (acos(bodyUp.x * up.x + bodyUp.y * up.y + bodyUp.z * up.z) * kDegreesPerRadian > physics->scamperToWalkAngle)
            *next = SS_SCAMPER_STOP;
    }
}
