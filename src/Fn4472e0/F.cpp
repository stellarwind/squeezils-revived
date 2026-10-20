// SQZ.EXE 0x4472e0 (cdecl): BounceBodies - one-dimensional elastic collision of two physical bodies along their motion:
// new velocities from the masses, then each body moves on in the new direction with its speed scaled by its bounciness.
// The second body's velocity uses (ma - mb) as well, as in the original.
struct V3 {
    float x, y, z;
    V3() {}
    ~V3();                                                  // 0x401660
    V3 operator*(float s) const;                            // 0x401b60
    V3 operator+(const V3 &o) const;                        // 0x403070
    V3 operator/(float s) const;                            // 0x420100
    float Normalize();                                      // 0x47d630
};
struct PhysicalBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual V3 *Direction();                                // slot 4 (+0x10)
    virtual float Speed();                                  // slot 5 (+0x14)
    virtual float Mass();                                   // slot 6 (+0x18)
    virtual void v7(); virtual void v8(); virtual void v9();
    virtual void SetMotion(V3 *direction, float speed);     // slot 10 (+0x28)
    char pad[0xbc - 4];
    float bounciness;                                       // 0xbc
};

void BounceBodies(PhysicalBody *a, PhysicalBody *b)
{
    float ma = a->Mass();
    float mb = b->Mass();
    V3 va = *a->Direction() * a->Speed();
    V3 vb = *b->Direction() * b->Speed();
    float sum = mb + ma;
    float diff = ma - mb;
    V3 na = (vb * 2.0f * mb + va * diff) / sum;
    V3 nb = (va * 2.0f * ma + vb * diff) / sum;
    float sa = na.Normalize();
    float sb = nb.Normalize();
    a->SetMotion(&na, a->bounciness * sa);
    b->SetMotion(&nb, sb * b->bounciness);
}
