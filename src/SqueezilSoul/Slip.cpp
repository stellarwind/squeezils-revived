// SqueezilSoul::WouldSlip (0x452900): self-contained (vector locals with out-of-line dtors).
// 12 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
#include <math.h>
#include "compat41.h"

struct Matrix {
    float m[4][4];                          // rows: right, up, forward, position
};

struct Vector3f {
    float x, y, z;
    Vector3f() {}
    Vector3f(const float *v) { x = v[0]; y = v[1]; z = v[2]; }
    Vector3f(const Matrix *m, int row) { x = m->m[row][0]; y = m->m[row][1]; z = m->m[row][2]; }
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    Vector3f Cross(const Vector3f &o) const;    // 0x41a350
    float Length() const;                   // 0x47d5f0
    float Normalize();                      // 0x47d630
};

struct Contact;

struct NavSurfaces {
    int IsSlippery(const Contact *c);       // 0x40f770
};

struct Place {
    NavSurfaces *Surfaces();                // 0x439b60
};

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2(); virtual void b3();
    virtual void b4(); virtual void b5();
    virtual float Mass();                   // slot 6, +0x18
};

struct SqueezilPhysics {
    float gravity[3];                       // 0x0
    char  pad0c[0x20 - 0xc];
    float staticFriction;                   // 0x20
    char  pad24[0x268 - 0x24];
    float clawGrip;                         // 0x268
    float crouchGrip;                       // 0x26c
};

enum { SS_RUN = 4, SS_SCAMPER = 9, SS_SCAMPER_STOP = 10 };

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
    char pad04[0x2c - 4];
    int  canSlip;                           // 0x2c
    char pad30[0x48 - 0x30];
    SqueezilPhysics *physics;               // 0x48
    char pad4c[0x70 - 0x4c];
    Place *place;                           // 0x70

    int WouldSlip(float push, const Matrix *m, const Contact *contact);
};

// @00452900 Whether a push along the surface (plus the slope's own pull) beats the grip: static friction on the weight pressing
// into the surface, plus ClawGrip, plus CrouchGrip when scampering. On a wall or ceiling nothing presses in and the whole weight
// pulls; running there always slips. A slippery surface type always slips; with canSlip cleared nothing does.
int SqueezilSoul::WouldSlip(float push, const Matrix *m, const Contact *contact)
{
    float staticFriction = physics->staticFriction;
    float clawGrip = physics->clawGrip;
    float crouchGrip = physics->crouchGrip;
    int current = GetState();
    float mass = Body()->Mass();
    Vector3f gravity(physics->gravity);
    Vector3f up(m, 1);
    Vector3f downhill;
    if (place->Surfaces()->IsSlippery(contact))
        return 1;
    if (!canSlip)
        return 0;
    float into = up.z * gravity.z + up.y * gravity.y + up.x * gravity.x;
    float sign = into < 0.0f ? -1.0f : 1.0f;
    int wall = sign == 1.0f || fabs(into) < 0.0001;
    if (GetState() == SS_RUN && wall)
        return 1;
    float pressing, pulling;
    if (!wall) {
        pressing = -(into * mass);
        Vector3f side = gravity.Cross(up);
        side.Normalize();
        downhill = side.Cross(up);
        pulling = -((downhill.z * gravity.z + downhill.y * gravity.y + downhill.x * gravity.x) * mass);
    } else {
        pulling = gravity.Length() * mass;
        pressing = 0.0f;
    }
    float grip = staticFriction * pressing + clawGrip;
    if (current == SS_SCAMPER || current == SS_SCAMPER_STOP)
        grip += crouchGrip;
    if (fabs(push) * mass + pulling > grip)
        return 1;
    return 0;
}
