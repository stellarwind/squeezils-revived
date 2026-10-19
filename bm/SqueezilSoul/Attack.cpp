// SqueezilSoul::Attack (0x4560d0): self-contained (vector temporaries with out-of-line dtors).
#include "compat41.h"

struct Matrix {
    float m[4][4];                          // rows: right, up, forward, position
};

struct Vector3f {
    float x, y, z;
    Vector3f(const float *v) { x = v[0]; y = v[1]; z = v[2]; }
    Vector3f(const Matrix *m, int row) { x = m->m[row][0]; y = m->m[row][1]; z = m->m[row][2]; }
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    Vector3f operator*(float s) const;      // 0x401b60
    Vector3f operator+(const Vector3f &o) const;    // 0x403070
    float Normalize();                      // 0x47d630
};

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix *GetMatrix();            // slot 3, +0xc
    virtual float  *Direction();            // slot 4, +0x10
    virtual float Speed();                  // slot 5, +0x14
    virtual void b6(); virtual void b7(); virtual void b8(); virtual void b9();
    virtual void SetVelocity(Vector3f *dir, float speed);  // slot 10, +0x28
    virtual void b11(); virtual void b12(); virtual void b13(); virtual void b14(); virtual void b15();
    virtual void b16(); virtual void b17(); virtual void b18(); virtual void b19();
    virtual int  GetMode();                 // slot 20, +0x50
    virtual void SetMode(int clip, int blend, int flag);   // slot 21, +0x54
    virtual int  ClipEnded();               // slot 22, +0x58
};

struct LeverMotion {
    char  pad[0x14];
    float minSpeed;                         // 0x14
    float pushIncrement;                    // 0x18
};

struct GroundControl {
    float forward;
    float turn;
    LeverMotion lever;                      // 0x8
};

struct InputState {
    char pad[0x68];
    int  attack;                            // 0x68
};
InputState *GetInput();                     // 0x4206b0

enum { SS_SKID = 5, SS_NONE = 24 };
enum { MODE_SLIDE_1 = 0x1d, MODE_SLIDE_2 = 0x1e, MODE_SLIDE_3 = 0x1f };

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
    virtual void s25(); virtual void s26();
    virtual int  GroundStateForSpeed(float speed); // slot 27, +0x6c
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31();
    virtual void s32(); virtual void s33(); virtual void s34(); virtual void s35();
    virtual void s36(); virtual void s37(); virtual void s38(); virtual void s39();
    virtual void s40(); virtual void s41(); virtual void s42(); virtual void s43();
    virtual void Slip(GroundControl *c, int next); // slot 44, +0xb0
    char pad04[0x44 - 4];
    int  frozen;                            // 0x44

    void Attack(GroundControl *c, int next);
};

// @004560d0 The sliding attack: a push (slide piece 1, PushIncrement more speed), the slide (piece 2) with another push when
// the attack is still held and the speed fell below MinSpeed, and the stop (piece 3). It moves like a slip throughout.
void SqueezilSoul::Attack(GroundControl *c, int next)
{
    int mode = Body()->GetMode();
    const LeverMotion &l = c->lever;
    switch (mode) {
    case MODE_SLIDE_1:
        if (Body()->ClipEnded()) {
            if (frozen == 0) {
                float speed = Body()->Speed();
                Vector3f direction(Body()->Direction());
                Vector3f forward(Body()->GetMatrix(), 2);
                direction = direction * speed + forward * l.pushIncrement;
                speed = direction.Normalize();
                Body()->SetVelocity(&direction, speed);
            }
            Body()->SetMode(MODE_SLIDE_2, 0, 0);
        }
        break;
    case MODE_SLIDE_2:
        if (GetInput()->attack && frozen == 0) {
            if (Body()->Speed() < l.minSpeed)
                Body()->SetMode(MODE_SLIDE_1, 0, 0);
        } else {
            Body()->SetMode(MODE_SLIDE_3, 0, 0);
        }
        break;
    case MODE_SLIDE_3:
        if (Body()->ClipEnded()) {
            SetState(next == SS_SKID ? next : GroundStateForSpeed(Body()->Speed()), 1);
            return;
        }
        break;
    }
    Slip(c, SS_NONE);
}
