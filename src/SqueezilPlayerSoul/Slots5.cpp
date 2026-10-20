// SqueezilPlayerSoul slot at SQZ.EXE 0x4564b0 (ret 8): when the body is in mode 0x1c and asks for it, run slot 24 with 0 and then with the given state.
struct Body {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17();
    virtual void s18(); virtual void s19();
    virtual int Slot20();                                // +0x50
    virtual void Slot21(int a, int b, int c);            // +0x54
    virtual int Slot22();                                // +0x58
};
struct Soul5 {
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
    virtual void s22();
    virtual void s23();
    virtual void Slot24(int a, int b);
    void Run(int unused, int state);
};

void Soul5::Run(int unused, int state)
{
    if (GetBody()->Slot20() == 0x1c && GetBody()->Slot22()) {
        Slot24(0, 0);
        if (state != 0x18)
            Slot24(state, 0);
    }
}
