// SqueezilSoul::StartJump (0x452be0): self-contained (vector and matrix locals with out-of-line dtors).
#include "compat41.h"

class String {
public:
    String(const char *s);                  // 0x438380
    ~String();                              // 0x4383d0
    char *rep;
};

struct Vector3f {
    float x, y, z;
    Vector3f() {}
    Vector3f(const Vector3f &o);            // 0x4183c0
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    Vector3f operator*(float s) const;      // 0x401b60
    Vector3f operator+(const Vector3f &o) const;    // 0x403070
    float Normalize();                      // 0x47d630
};

struct Matrix {
    float m[4][4];
    Vector3f *Position();                   // 0x41faf0
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
    Vector3f &Row(int i) { return *(Vector3f *)&v[i * 4]; }
    void SetTranslation(const Vector3f &p); // 0x47e330
};

void PlayEffect(String name, Vector3f at); // 0x44a250
void DebugLog(const char *format, ...);     // 0x419360

struct Clock {
    int Ticks();                            // 0x41ebf0
    int Frame();                            // 0x41ec10
};
Clock *GetClock();                          // 0x4206d0

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix   *GetMatrix();          // slot 3, +0xc
    virtual Vector3f *Direction();          // slot 4, +0x10
    virtual float Speed();                  // slot 5, +0x14
    virtual void b6(); virtual void b7(); virtual void b8();
    virtual void SetMatrix(Matrix16 *m);    // slot 9, +0x24
    virtual void SetVelocity(Vector3f *dir, float speed);  // slot 10, +0x28
    virtual void b11(); virtual void b12(); virtual void b13(); virtual void b14();
    virtual int  IsOnGround();              // slot 15, +0x3c
};

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual void s18(); virtual void s19(); virtual void s20();
    virtual void ResetSurface();            // slot 21, +0x54
    char pad04[0x7c - 4];
    float jumpStartY;                       // 0x7c

    void StartJump(float upSpeed, float fwdSpeed, float upDistance);
};

// @00452be0 Launches the squeezil off the ground: up and forward speeds are added to its velocity, and it is lifted by
// upDistance so it starts clear of the surface. Writes a trace to the debug output.
void SqueezilSoul::StartJump(float upSpeed, float fwdSpeed, float upDistance)
{
    if (Body()->IsOnGround()) {
        Vector3f right, up, forward, upPush, fwdPush, velocity, lift, position;
        Matrix16 m = *Body()->GetMatrix();
        right = m.Row(0);
        up = m.Row(1);
        forward = m.Row(2);
        float speed = Body()->Speed();
        velocity = *Body()->Direction();
        upPush = up * upSpeed;
        fwdPush = forward * fwdSpeed;
        velocity = velocity * speed + upPush + fwdPush;
        speed = velocity.Normalize();
        position = m.Row(3);
        DebugLog("Starting jump, upSpeed %f, fwdSpeed %f, upDist %f.\n", upSpeed, fwdSpeed, upDistance);
        DebugLog("%d\t%d\t%f\t%f\n", GetClock()->Frame(), GetClock()->Ticks(), position.x, position.y);
        jumpStartY = position.y;
        lift = up * upDistance;
        position = position + lift;
        Body()->SetVelocity(&velocity, speed);
        m.SetTranslation(position);
        Body()->SetMatrix(&m);
        PlayEffect("jumpEffect", *Body()->GetMatrix()->Position());
        ResetSurface();
        DebugLog("%d\t%d\t%f\t%f\n", GetClock()->Frame(), GetClock()->Ticks(), position.x, position.y);
    }
}
