// 24 spare declarations (symbol count, see padscan.sh)
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
void PadDecl24();
// SqueezilPlayerSoul slot at SQZ.EXE 0x452bb0: if the soul takes the flag (slot 23), clear it, copy the physics value and re-run slot 21.
struct Physics { char pad[0x274]; int value; };
struct Soul {
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
    virtual void Slot21();
    virtual void s22();
    virtual int Slot23(int a);
    virtual void Slot24(int a, int b);
    char pad[0x48 - 4];
    Physics *physics;                                    // 0x48
    char pad2[0x78 - 0x4c];
    int field78;                                         // 0x78
    void Run();
};

void Soul::Run()
{
    if (Slot23(0xb)) {
        Slot24(0xb, 0);
        field78 = physics->value;
        Slot21();
    }
}
