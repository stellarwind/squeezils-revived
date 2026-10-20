// 23 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
void PadDecl18();
void PadDecl19();
void PadDecl20();
void PadDecl21();
void PadDecl22();
void PadDecl23();
// WrapSoul::WrapA (0x4045b0): state, virtual act, behaviour run.
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

