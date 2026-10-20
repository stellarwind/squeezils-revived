// SqueezilSoul::Turn (0x4548a0): self-contained (by-value matrix, contact and direction locals with out-of-line dtors).
#include "compat41.h"

struct Matrix16 {                           // a body matrix held by value (64 bytes)
    float v[16];
    Matrix16(const Matrix16 &o)
    {
        v[0] = o.v[0];
        v[1] = o.v[1];
        v[2] = o.v[2];
        v[3] = o.v[3];
        v[4] = o.v[4];
        v[5] = o.v[5];
        v[6] = o.v[6];
        v[7] = o.v[7];
        v[8] = o.v[8];
        v[9] = o.v[9];
        v[10] = o.v[10];
        v[11] = o.v[11];
        v[12] = o.v[12];
        v[13] = o.v[13];
        v[14] = o.v[14];
        v[15] = o.v[15];
    }
    ~Matrix16();                            // 0x402310
};

struct Contact {                            // 0x48 bytes
    int a, b;
    Matrix16 frame;                         // 0x8
    ~Contact();                             // 0x4031e0
};

struct Vector3f {
    float x, y, z;
    Vector3f(const float *v) { x = v[0]; y = v[1]; z = v[2]; }
    ~Vector3f();                            // 0x401660
};

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix16 *GetMatrix();          // slot 3, +0xc
    virtual float   *Direction();           // slot 4, +0x10
    virtual float Speed();                  // slot 5, +0x14
    virtual void b6();
    virtual Contact *GetContact();          // slot 7, +0x1c
    virtual void b8();
    virtual void SetMatrix(Matrix16 *m);    // slot 9, +0x24
    virtual void SetVelocity(Vector3f *dir, float speed);  // slot 10, +0x28
    virtual void b11(); virtual void b12(); virtual void b13();
    virtual void SetContact(Contact *c);    // slot 14, +0x38
};

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void s24(); virtual void s25(); virtual void s26(); virtual void s27();
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31();
    virtual void RotateOnSurface(Matrix16 *m, Vector3f *dir, Contact *c, float radians);   // slot 32, +0x80

    void Turn(float radians);
};

// @004548a0 Turn on the spot: rotate the body's frame about the surface normal, keeping speed and contact.
void SqueezilSoul::Turn(float radians)
{
    Matrix16 m = *Body()->GetMatrix();
    Contact contact = *Body()->GetContact();
    Vector3f direction(Body()->Direction());
    float speed = Body()->Speed();
    RotateOnSurface(&m, &direction, &contact, radians);
    Body()->SetMatrix(&m);
    Body()->SetVelocity(&direction, speed);
    Body()->SetContact(&contact);
}
