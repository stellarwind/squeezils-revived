// SqueezilPlayerSoul::JumpPressed (0x41f070): runs on the second base subobject (this = soul + 0x80).
struct PBody {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual void SetAngles(float a, float b, float c);   // 11, +0x2c
};
struct PhysX { char pad[0x198]; float upSpeed; float fwdSpeed; float upDistance; char pad2[0x288 - 0x1a4]; int unknown288; };
struct PlayerSoulX {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual void s11();
    virtual void s12();
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual PBody *Body();                         // 17, +0x44
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void s22();
    virtual void s23();
    virtual void SetState(int state, int blend);   // 24, +0x60
    virtual int GetState();                        // 25, +0x64
    virtual void s26();
    virtual void s27();
    virtual void s28();
    virtual void s29();
    virtual void s30();
    virtual void s31();
    virtual void s32();
    virtual void s33();
    virtual void s34();
    virtual void s35();
    virtual void s36();
    virtual void StartJump(float up, float fwd, float dist);   // 37, +0x94
    char pad0[0x44 - 4];
    int frozen;               // +0x44
    PhysX *physics;           // +0x48
    char pad1[0x50 - 0x4c];
    float hopStrength;        // +0x50
};

struct PlayerSoulSub {
    char pad[4];
    void JumpPressed();
};

void PlayerSoulSub::JumpPressed()
{
    char *self = (char *)this;
    if (*(int *)(self - 0x3c) || !(*(PhysX **)(self - 0x38))->unknown288)
        return;
    PlayerSoulX *s = (PlayerSoulX *)(self - 0x80);
    switch (s->GetState()) {
    case 0:
        s->SetState(13, 0);
        s->Body()->SetAngles(0.0f, 0.0f, 0.0f);
        *(float *)(self - 0x30) = -1.0f;
        break;
    case 3:
    case 6:
    case 7:
        s->SetState(13, 0);
        *(float *)(self - 0x30) = 1.0f;
        break;
    case 4:
    case 9:
    case 10:
        s->StartJump((*(PhysX **)(self - 0x38))->upSpeed, (*(PhysX **)(self - 0x38))->fwdSpeed, (*(PhysX **)(self - 0x38))->upDistance);
        s->SetState(12, 0);
        break;
    case 12:
    case 14:
    case 15:
        s->SetState(17, 0);
        break;
    }
}

struct PlayerSoulY {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void SetState(int state, int blend);   // 24, +0x60
    virtual void s25(); virtual void s26(); virtual void s27(); virtual void s28();
    virtual int SpitState();                       // 29, +0x74
    virtual void SetSpitStrength(float s);         // 30, +0x78
};

struct PlayerSoulSub2 {
    char pad[4];
    void JumpReleased(float strength);
    void SpitPressed();
    void SpitReleased(float strength);
};

void PlayerSoulSub2::JumpReleased(float strength)
{
    char *self = (char *)this;
    if (!*(int *)(self - 0x3c) && (*(PhysX **)(self - 0x38))->unknown288)
        *(float *)(self - 0x30) = strength;
}

void PlayerSoulSub2::SpitPressed()
{
    PlayerSoulY *s = (PlayerSoulY *)((char *)this - 0x80);
    s->SetState(s->SpitState(), 0);
}

void PlayerSoulSub2::SpitReleased(float strength)
{
    ((PlayerSoulY *)((char *)this - 0x80))->SetSpitStrength(strength);
}
