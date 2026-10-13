// PaceBehavior::Execute (0x402cd0): walk to one end, wait, turn around by an angle, walk back; seven-state machine.
#define V3_COPYCTOR
#include "Behavior2/Activity.h"
#include "Vector3f/Vector3f.h"

struct PaceMat {                                         // the 64-byte matrix of the transform code (rows of 3 + w)
    float m[16];
    PaceMat() {}
    ~PaceMat();                                          // empty, out of line (0x402310)
    PaceMat(const PaceMat &o)
    {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
    }
    PaceMat &operator=(const PaceMat &o)
    {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
        return *this;
    }
    PaceMat &TurnY(float angle);                         // 0x47dd20
    PaceMat operator*(const PaceMat &o) const;           // 0x47dea0
};
struct PaceSub {
    virtual int Run(int a);                              // slot 0
    virtual int Execute(int a);                          // slot 1
    char pad[0x2b4 - 4];
    unsigned flags;                                      // 0x2b4
    int pad2b8;
    Vector3f dest;                                       // 0x2bc
};
struct PaceBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual PaceMat *Transform();                        // slot 3 (+0xc)
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14();
    virtual int OnGround();                              // slot 15 (+0x3c)
};
struct PaceClock { int Elapsed(); };                     // 0x41ec00
PaceClock *GetClock();                                   // 0x4206d0 (cdecl)

struct PaceBehavior {
    virtual void s0();
    virtual int Execute(int a, int b);                   // slot 1
    PaceBody *body;                                      // 0x04
    void *soul;                                          // 0x08
    PaceSub *current;                                    // 0x0c
    int timer;                                           // 0x10
    int mode;                                            // 0x14
    PaceSub *walkA;                                      // 0x18
    PaceSub *walkB;                                      // 0x1c
    PaceSub *idle;                                       // 0x20
    float turnDegrees;                                   // 0x24
    float waitA;                                         // 0x28
    float waitB;                                         // 0x2c
};

int PaceBehavior::Execute(int a, int b)
{
    int m = mode;
    switch (m) {
    case 0:
        m++;
        mode = m;
        break;
    case 1:
        m++;
        current = idle;
        timer = (int)(waitB * 1000.0f);
        mode = m;
        break;
    case 2:
        timer -= GetClock()->Elapsed();
        if (timer <= 0) {
            current = walkA;
            timer = (int)(waitA * 1000.0f);
            mode++;
        }
        break;
    case 3:
        timer -= GetClock()->Elapsed();
        if (timer <= 0) {
            current = idle;
            timer = (int)(waitB * 1000.0f);
            mode++;
        }
        break;
    case 4:
        timer -= GetClock()->Elapsed();
        if (timer <= 0) {
            PaceMat orient = *body->Transform();
            PaceMat turn;
            Vector3f p(*(Vector3f *)&orient.m[12]);
            Vector3f dest;
            turn.TurnY(turnDegrees * 0.0174533f);
            orient = turn * orient;
            dest = p + (*(Vector3f *)&orient.m[8]) * 10.0f;
            PaceSub *w = current = walkB;
            w->dest = dest;
            mode++;
        }
        break;
    case 5:
        if (!(walkB->flags & 0x7fffffff)) {
            m++;
            mode = m;
        }
        break;
    case 6:
        mode = 1;
        break;
    }
    if (body->OnGround())
        return current->Execute(a);
    return current->Run(b);
}
