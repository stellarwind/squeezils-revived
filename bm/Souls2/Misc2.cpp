// 0x402340 (16 word copy returning this), 0x401be0 (stand step), 0x4045b0 / 0x4045f0 (two virtual-call wrappers).
struct Mi16 {
    int m[16];
    Mi16 *Assign(const int *src);
};

Mi16 *Mi16::Assign(const int *src)
{
    m[0] = src[0];
    m[1] = src[1];
    m[2] = src[2];
    m[3] = src[3];
    m[4] = src[4];
    m[5] = src[5];
    m[6] = src[6];
    m[7] = src[7];
    m[8] = src[8];
    m[9] = src[9];
    m[10] = src[10];
    m[11] = src[11];
    m[12] = src[12];
    m[13] = src[13];
    m[14] = src[14];
    m[15] = src[15];
    return this;
}

struct SBod { virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual float Speed(); };
struct SSoul {
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
    virtual void s24();
    virtual int GetState();   // 25
};
extern float kStillSpeed2;   // 0x49d014

struct StandAct {
    int pad;
    SBod *body;               // +4
    SSoul *soul;              // +8
    int StepStand(int *out);
};

int StandAct::StepStand(int *out)
{
    int wanted = 0x18;
    out[1] = 0;
    out[0] = 0;
    if (soul->GetState()) {
        if (body->Speed() <= kStillSpeed2)
            wanted = 0;
    }
    return wanted;
}

struct Beh { virtual void s0(); virtual int Run(int a, int b); };   // slot 1
struct WrapSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void s24();
    virtual int GetState();                      // 25, +0x64
    virtual void s26(); virtual void s27(); virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31(); virtual void s32(); virtual void s33();
    virtual void s34(); virtual void s35(); virtual void s36(); virtual void s37(); virtual void s38(); virtual void s39(); virtual void s40();
    virtual void ActA(int state, int arg);       // 41, +0xa4
    virtual void ActB(int state, int arg);       // 42, +0xa8
    char pad[0xec - 4];
    Beh *behaviour;                              // +0xec
    void WrapA(int arg, int *out);
    void WrapB(int arg, int *out);
};

void WrapSoul::WrapA(int arg, int *out)
{
    int st = GetState();
    ActA(st, arg);
    *out = behaviour->Run(arg, 0);
}

void WrapSoul::WrapB(int arg, int *out)
{
    int st = GetState();
    ActB(st, arg);
    *out = behaviour->Run(0, arg);
}
