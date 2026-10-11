// SqueezilSoul::DyingInAir (0x456530) and AnimatedBody::SetRootTransform (0x41e230).
#include "Renderer2/Renderer2.h"
struct AirCtl { float pitch; float turn; };
struct DieSoul {
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
    virtual void s17();
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void s22();
    virtual void s23();
    virtual void SetState(int s, int blend);   // 24, +0x60
    virtual int GetState();                    // 25, +0x64
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
    virtual void s37();
    virtual void s38();
    virtual void s39();
    virtual void s40();
    virtual void s41();
    virtual void GetAirMotion(int state, AirCtl *c);   // 42, +0xa8
    virtual void s43();
    virtual void s44();
    virtual void Handle(AirCtl *c, int next);          // 45, +0xb4
    char pad0[0x38 - 4];
    int stateAtDeath;         // +0x38
    void DyingInAir(AirCtl *c, int next);
};

void DieSoul::DyingInAir(AirCtl *c, int next)
{
    GetAirMotion(stateAtDeath, c);
    c->pitch = 999.0f;
    c->turn = 0.0f;
    Handle(c, 0x18);
    if (GetState() == 0x13) {
        SetState(0x15, 0);
        SetState(0x16, 0);
    }
}

struct AnimBody {
    char pad[0x4c];
    LPDIRECT3DRMFRAME rootFrame;   // +0x4c
    void SetRootTransform(LPDIRECT3DRMFRAME ref, const float *m);
};

void AnimBody::SetRootTransform(LPDIRECT3DRMFRAME ref, const float *m)
{
    rootFrame->SetPosition(ref, m[12], m[13], m[14]);
    rootFrame->SetOrientation(ref, m[8], m[9], m[10], m[4], m[5], m[6]);
}
