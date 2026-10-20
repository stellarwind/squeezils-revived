// 21 spare declarations (symbol count, see padscan.sh)
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
// SqueezilPlayerSoul slot at SQZ.EXE 0x456500 (ret 8): if the body asks for it (body slot 22), set field 8 and run slot 24 with 0x16.
struct Body {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17();
    virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21();
    virtual int Slot22();                                // +0x58
};
struct Soul3 {
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
    char pad[8 - 4];
    int field8;
    void Run(int a, int b);
};

void Soul3::Run(int a, int b)
{
    if (GetBody()->Slot22()) {
        field8 = 1;
        Slot24(0x16, 0);
    }
}
