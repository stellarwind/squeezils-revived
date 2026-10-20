// GoodieSoul::Launch (SQZ.EXE 0x459ed0, ret 0x50): thrown from a matrix in a direction at a speed.
struct V3 { float x, y, z; ~V3(); float Normalize(); };         // Normalize 0x47d630
struct Matrix16 { float m[16]; ~Matrix16(); };           // dtor 0x402310
struct GoodieBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetMatrix(Matrix16 *m);                 // slot 9 (+0x24)
    virtual void SetFacing(V3 *dir, float speed);        // slot 10 (+0x28)
};
struct GoodieSoul {
    void **vtbl, **grantVtbl;
    GoodieBody *body;                                    // 0x08
    char pad0c[4];
    int kind;                                            // 0x10
    int thrown;                                          // 0x14
    char pad18[4];
    float flightTime;                                    // 0x1c
    V3 direction;                                        // 0x20
    void Launch(Matrix16 from, V3 course, float speed);
};

void GoodieSoul::Launch(Matrix16 from, V3 course, float speed)
{
    body->SetMatrix(&from);
    direction.x = course.x;
    direction.y = course.y;
    direction.z = course.z;
    direction.Normalize();
    body->SetFacing(&direction, speed);
    flightTime = 0.0f;
    thrown = 1;
}
