// SqueezilSoul::SetState (0x450320): flat layout, virtual calls by slot.
struct SBody {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void SetOnGround(int onGround);          // slot 16, +0x40
    virtual void s17(); virtual void s18();
    virtual void UpdateOrientation();                // slot 19, +0x4c
    virtual void s20();
    virtual void SetMode(int clip, int blend, int flag);   // slot 21, +0x54
};

extern int kStateOnGround[];     // 0x49ed40
extern int kStateClip[];         // 0x49f5e8

struct SoulFlat {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SBody *Body();                           // slot 17, +0x44
    char pad0[0x34 - 4];
    int limitLanding;                                // +0x34
    int pad1;
    int state;                                       // +0x3c
    int previousState;                               // +0x40
    char pad2[0x50 - 0x44];
    float hopStrength;                               // +0x50
    float spitStrength;                              // +0x54
    char pad3[0x78 - 0x58];
    float tumbleTime;                                // +0x78
    void SetState(int newState, int blend);
};

void SoulFlat::SetState(int newState, int blend)
{
    if (newState == state || newState == 24)
        return;
    previousState = state;
    state = newState;
    if (newState != 22)
        Body()->SetMode(kStateClip[newState], blend, 0);
    int wasOnGround = kStateOnGround[previousState];
    int onGround = kStateOnGround[newState];
    Body()->SetOnGround(onGround);
    if (wasOnGround && !onGround)
        Body()->UpdateOrientation();
    if (state == 19) {
        limitLanding = 0;
        if (previousState != 11)
            limitLanding = 1;
    }
    if (state != 11)
        tumbleTime = 0.0f;
    if (state != 13 && state != 14)
        hopStrength = -1.0f;
    if (state != 16)
        spitStrength = -1.0f;
}
