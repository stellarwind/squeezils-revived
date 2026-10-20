// SqueezilSoul slot at SQZ.EXE 0x455c00 (ret 8): base slot, then in state 0x13 report the body's contact.
struct Float3 { float x, y, z; };
struct Cont { char w[0x48]; Cont(const Cont &c); ~Cont(); };   // copy ctor 0x452840, dtor 0x4031e0
struct Body {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual float *GetMatrix();                          // slot 3 (+0xc)
    virtual Float3 *GetFacing();                         // slot 4 (+0x10)
    virtual float GetFacingValue();                      // slot 5 (+0x14)
    virtual void s6();
    virtual Cont *GetContact();                       // slot 7 (+0x1c)
    virtual void s8(); virtual void s9();
    virtual void SetFacing(Float3 *dir, float speed);    // slot 10 (+0x28)
    virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17();
    virtual void s18(); virtual void s19(); virtual void s20();
    virtual void Slot21(int a, int b, int c);            // +0x54
};
struct SoulA {
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
    virtual struct Body *GetBody();
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void Slot22(struct Cont c);
    virtual void s23();
    virtual void Slot24(int a, int b);
    virtual int GetState();
    virtual void s26();
    void Base(int a, int b);                             // 0x455c90 (ret 8)
    void Run(int a, int b);                              // 0x455c00
};

void SoulA::Run(int a, int b)
{
    Base(a, b);
    if (GetState() == 0x13)
        Slot22(*GetBody()->GetContact());
}
