// GoodieSoul::Hit (slot 9, SQZ.EXE 0x459a80, ret 0xc): a loose goodie at rest flies off the way the hitter was going; it is no longer thrown.
struct V3 { float x, y, z; ~V3(); V3 operator*(float s) const; float Length() const; };   // dtor 0x401660, 0x401b60, 0x47d5f0
struct GoodieBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual V3 *GetFacing();                             // slot 4 (+0x10)
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void SetFacing(const V3 &dir, float speed);  // slot 10 (+0x28)
};
struct Hitter {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual V3 *GetFacing();                             // slot 4 (+0x10)
    virtual float GetFacingValue();                      // slot 5 (+0x14)
};
struct Sim { Hitter *GetActor(int id); };                // 0x414050
Sim *GetSimulation();                                    // 0x420690 (cdecl)
struct GoodieSoul {
    void **vtbl, **grantVtbl;
    GoodieBody *body;                                    // 0x08
    char pad0c[4];
    int kind;                                            // 0x10
    int thrown;                                          // 0x14
    char pad18[4];
    float flightTime;                                    // 0x1c
    V3 direction;                                        // 0x20
    void Hit(int knock, int unused, int byId);
};

void GoodieSoul::Hit(int knock, int unused, int byId)
{
    if (knock && kind == 0) {
        if (body->GetFacing()->Length() < 0.01f) {
            Hitter *hitter = GetSimulation()->GetActor(byId);
            body->SetFacing(*hitter->GetFacing() * -1.0f, hitter->GetFacingValue());
        }
        V3 *facing = body->GetFacing();
        direction.x = facing->x;
        direction.y = facing->y;
        direction.z = facing->z;
        flightTime = 0.0f;
    }
    thrown = 0;
}
