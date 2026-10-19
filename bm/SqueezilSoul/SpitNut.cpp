// SqueezilSoul::SpitNut (0x4530f0): self-contained (vector and matrix values with out-of-line dtors and copy ctors).
#include "compat41.h"

struct Vector3f {
    float x, y, z;
    Vector3f() {}
    Vector3f(const struct Matrix *m, int row);
    Vector3f(const Vector3f &o);            // 0x4183c0
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    Vector3f operator*(float s) const;      // 0x401b60
    Vector3f operator+(const Vector3f &o) const;    // 0x403070
    float Normalize();                      // 0x47d630
};

struct Matrix {
    float m[4][4];                          // rows: right, up, forward, position
    Vector3f &Row(int i) { return *(Vector3f *)m[i]; }
};

inline Vector3f::Vector3f(const Matrix *m, int row) { x = m->m[row][0]; y = m->m[row][1]; z = m->m[row][2]; }

struct ThrowMatrix {                        // a body matrix passed by value (64 bytes)
    float v[16];
    ThrowMatrix(const Matrix &o)
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
    ThrowMatrix(const ThrowMatrix &o);      // 0x4099e0
    ~ThrowMatrix();                         // 0x402310
    void SetTranslation(const Vector3f &p); // 0x47e330
};

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix   *GetMatrix();          // slot 3, +0xc
    virtual Vector3f *Direction();          // slot 4, +0x10
    virtual float Speed();                  // slot 5, +0x14
    virtual void b6(); virtual void b7(); virtual void b8(); virtual void b9();
    virtual void b10(); virtual void b11(); virtual void b12(); virtual void b13(); virtual void b14();
    virtual int  IsOnGround();              // slot 15, +0x3c
};

struct NutList {
    virtual int  At(int i);                 // slot 0
    virtual void a1();
    virtual void RemoveAt(int i);           // slot 2
};

struct Inventory {
    int      Count(const char *kind);       // 0x416300
    NutList *Items(const char *kind);       // 0x4163d0
};

struct Possessions {
    Inventory *GetInventory();              // 0x448990
};

struct PlaceGoodies {
    void Throw(int item, ThrowMatrix at, Vector3f velocity, float speed);  // 0x44dc30
};

struct Place {
    PlaceGoodies *Goodies();                // 0x439b50
};

extern const char kSpitNut[];               // 0x4b9334

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual Possessions *GetPossessions();  // slot 18, +0x48
    char pad04[0x70 - 4];
    Place *place;                           // 0x70

    void SpitNut(float speed);
};

// @004530f0 Throws the first spit nut forward from the mouth (a little up and forward of the body on the ground, a step ahead
// in the air), with the squeezil's own velocity added.
void SqueezilSoul::SpitNut(float speed)
{
    if (GetPossessions()->GetInventory()->Count(kSpitNut) == 0)
        return;
    NutList *nuts = GetPossessions()->GetInventory()->Items(kSpitNut);
    Vector3f right(Body()->GetMatrix(), 0);
    Vector3f up(Body()->GetMatrix(), 1);
    Vector3f forward(Body()->GetMatrix(), 2);
    Vector3f velocity = *Body()->Direction() * Body()->Speed();
    velocity = velocity + Body()->GetMatrix()->Row(2) * speed;
    Vector3f from;
    if (Body()->IsOnGround()) {
        from = Body()->GetMatrix()->Row(3);
        from = from + up * 0.2f + forward * 0.1f;
    } else {
        from = Body()->GetMatrix()->Row(3);
        from = from + forward * 1.0f;
    }
    ThrowMatrix at = *Body()->GetMatrix();
    at.SetTranslation(from);
    float throwSpeed = velocity.Normalize();
    place->Goodies()->Throw(nuts->At(0), at, velocity, throwSpeed);
    nuts->RemoveAt(0);
}
