// SqueezilSoul::JumpFlight, SQZ.EXE 0x455c90.
#include "compat41.h"
#include <string.h>

struct Vector3f {
    float x, y, z;
    Vector3f(const float *p) { x = p[0]; y = p[1]; z = p[2]; }
    ~Vector3f();                            // 0x401660
    Vector3f operator*(float s) const;      // 0x401b60
    Vector3f operator+(const Vector3f &v) const; // 0x403070
    float Normalize();                     // 0x47d630
};

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual float *GetMatrix();             // +0x0c
    virtual float *Direction();             // +0x10
    virtual float Speed();                 // +0x14
    virtual void b6(); virtual void b7(); virtual void b8(); virtual void b9();
    virtual void SetVelocity(Vector3f *v, float speed); // +0x28
};

struct Clock { unsigned long FrameMilliseconds(); }; // 0x41ec00
Clock *GetClock();                          // 0x4206d0

struct JumpAirControl {
    float pitch;
    float pad04[4];
    float maxLinearSpeed, linearAccel;      // 0x14, 0x18
    float dragX, dragY, dragZ, aoa;          // 0x1c..0x28
    float gravityFactor;                   // 0x2c
    float rest[13];
};

struct PhysSet {
    char pad00[0x10];
    float downY;
    float pad14;
    float gravityStrength;                 // 0x18
    char pad1c[0x13c - 0x1c];
    float tumble[23];                      // 0x13c
    float SetGravity(float strength);      // 0x446dd0
};

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // +0x44
    virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21();
    virtual void s22(); virtual void s23(); virtual void s24(); virtual void s25();
    virtual void s26(); virtual void s27(); virtual void s28(); virtual void s29();
    virtual void s30(); virtual void s31(); virtual void s32(); virtual void s33();
    virtual void s34(); virtual void s35(); virtual void s36(); virtual void s37();
    virtual void s38(); virtual void s39(); virtual void s40(); virtual void s41();
    virtual void s42(); virtual void s43(); virtual void s44();
    virtual void Fly(JumpAirControl *c, int next); // +0xb4
    char pad04[0x48 - 4];
    PhysSet *physics;                       // 0x48
    float SpeedStep(float current, float accel, float decel, float target, float seconds); // 0x451630
    void JumpFlight(JumpAirControl *c, int next);
};

void SqueezilSoul::JumpFlight(JumpAirControl *c, int next)
{
    // The original tests the input sentinel and target sign as raw float bits.
    if (*(unsigned long *)&c->pitch != 0x4479c000UL) {
        Vector3f forward(Body()->GetMatrix() + 8);
        Vector3f direction(Body()->Direction());
        float speed = Body()->Speed();
        float seconds = GetClock()->FrameMilliseconds() * 0.001f;
        float target = c->maxLinearSpeed * c->pitch;
        float change = SpeedStep((direction.z * forward.z + direction.y * forward.y +
                                  direction.x * forward.x) * speed,
                                 c->linearAccel, c->linearAccel, target, seconds);
        float changeSign = change < 0.0f ? -1.0f : 1.0f;
        float targetSign = *(unsigned long *)&target > 0x80000000UL ? -1.0f : 1.0f;
        if (changeSign == targetSign) {
            Vector3f velocity = direction * speed + forward * change;
            speed = velocity.Normalize();
            Body()->SetVelocity(&velocity, speed);
        }
        c->pitch = 999.0f;
    }
    float gravity = physics->gravityStrength * physics->downY;
    physics->SetGravity(gravity * c->gravityFactor);
    float dragX = c->dragX, dragY = c->dragY, dragZ = c->dragZ, aoa = c->aoa;
    memcpy((char *)c + 8, physics->tumble, sizeof(physics->tumble));
    ((float *)c)[17] = dragX;
    ((float *)c)[15] = dragY;
    ((float *)c)[13] = dragZ;
    ((float *)c)[20] = aoa;
    Fly(c, next);
    physics->SetGravity(gravity);
}
