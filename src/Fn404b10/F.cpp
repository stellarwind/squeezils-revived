// SqueezilRoboSoul::SetBehavior (SQZ.EXE 0x404b10, slot 60, ret 8): replace the current behaviour by a new one of the
// given kind (0 pace, 4 chase, 6 death; 1, 2, 3, 5 are among Flee/SimpleFly/Spit/Wander), built with the soul's body and
// the kind's parameters. Kinds above 6 store an uninitialised pointer, as the original does.
struct BodyR;
struct SqueezilRoboSoulB;
struct Behavior { virtual ~Behavior(); };                  // slot 0: deleting destructor
struct PaceBehavior : Behavior {                            // 0x30
    char pad[0x2c];
    PaceBehavior(BodyR *b, SqueezilRoboSoulB *s, float a, float c, float d);   // 0x402930
};
struct Behavior1 : Behavior { char pad[0x20]; Behavior1(BodyR *b, SqueezilRoboSoulB *s); };   // 0x24, 0x4023b0
struct Behavior2 : Behavior { char pad[0x14]; Behavior2(BodyR *b, SqueezilRoboSoulB *s); };   // 0x18, 0x4033b0
struct Behavior3 : Behavior { char pad[0x1c]; Behavior3(BodyR *b, SqueezilRoboSoulB *s); };   // 0x20, 0x403650
struct ChaseBehavior : Behavior {                           // 0x28
    char pad[0x24];
    ChaseBehavior(BodyR *b, SqueezilRoboSoulB *s, float a, float c);          // 0x403b60
};
struct Behavior5 : Behavior { char pad[0x1c]; Behavior5(BodyR *b, SqueezilRoboSoulB *s); };   // 0x20, 0x403ed0
struct DeathBehavior : Behavior {                           // 0x24
    char pad[0x20];
    DeathBehavior(BodyR *b, SqueezilRoboSoulB *s, float a);                   // 0x4030f0
};

struct SqueezilRoboSoulB {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16();
    virtual BodyR *GetBody();                               // slot 17 (+0x44)
    virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24();
    virtual int GetAnimation();                             // slot 25 (+0x64)
    virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29();
    virtual void SetAnimationSpeed(float s);                // slot 30 (+0x78)
    char pad[0xec - 4];
    Behavior *behavior;                                     // 0xec
    int kind;                                               // 0xf0
    void SetBehavior(int newKind, float *values);
};

void SqueezilRoboSoulB::SetBehavior(int newKind, float *values)
{
    if (kind == newKind)
        return;
    Behavior *b;
    switch (newKind) {
    case 0: b = new PaceBehavior(GetBody(), this, values[0], values[1], values[2]); break;
    case 1: b = new Behavior1(GetBody(), this); break;
    case 2: b = new Behavior2(GetBody(), this); break;
    case 3: b = new Behavior3(GetBody(), this); break;
    case 4: b = new ChaseBehavior(GetBody(), this, values[0], values[1]); break;
    case 5: b = new Behavior5(GetBody(), this); break;
    case 6: b = new DeathBehavior(GetBody(), this, values[0]); break;
    }
    if (behavior) {
        if (kind == 4 && GetAnimation() == 16)
            SetAnimationSpeed(1.0f);
        delete behavior;
    }
    behavior = b;
    kind = newKind;
}
