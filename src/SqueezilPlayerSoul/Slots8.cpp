// SqueezilPlayerSoul slot at SQZ.EXE 0x456350 (ret 8): body modes 0x2b/0x2c/0x2d (a jump): 0x2b -> 0x2c, 0x2c idle, 0x2d lands; then state tail.
struct Vec;
struct Body {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual Vec *GetFacing();                       // slot 4 (+0x10)
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void Slot10(Vec *v, int a);                         // +0x28
    virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17();
    virtual void s18(); virtual void s19();
    virtual int Slot20();                                // +0x50
    virtual void Slot21(int a, int b, int c);            // +0x54
    virtual int Slot22();                                // +0x58
};
struct Sub { void Reset(); };                            // 0x456ef0
struct Soul8 {
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
    virtual int GetState();
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
    virtual void s42();
    virtual void s43();
    virtual void Slot44(int a, int b);
    char pad[0x10 - 4];
    Sub sub;                                             // 0x10
    void Run(int a, int b);
};

void Soul8::Run(int a, int b)
{
    int mode = GetBody()->Slot20();
    if (mode == 0x2b || mode == 0x2c) { if (mode == 0x2b && GetBody()->Slot22()) GetBody()->Slot21(0x2c, 0, 0); } else if (mode == 0x2d) { if (GetBody()->Slot22()) { GetBody()->Slot10(GetBody()->GetFacing(), 0); Slot24(0, 1); sub.Reset(); } return; } else return;
    Slot44(a, 0);
    if (GetState() == 0) {
        Slot24(8, 0);
        GetBody()->Slot21(0x2d, 0, 0);
    }
}
