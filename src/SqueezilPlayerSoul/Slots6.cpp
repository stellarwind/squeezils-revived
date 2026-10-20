// SqueezilPlayerSoul slot at SQZ.EXE 0x456430 (ret 8): a body in mode 0x1a switches to 0x1b; unless the stick is idle, slot 24 with 2 and stop the looping sound.
#include <math.h>
struct Body {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17();
    virtual void s18(); virtual void s19();
    virtual int Slot20();                                // +0x50
    virtual void Slot21(int a, int b, int c);            // +0x54
    virtual int Slot22();                                // +0x58
};
extern int g_loopingSound;                               // 0x4bd960
void StopLoopingSounds();                                // 0x44a8f0 (cdecl)
struct Control { float forward, turn; };
struct Soul6 {
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
    void Run(Control *c, int state);
};

void Soul6::Run(Control *c, int state)
{
    if (GetBody()->Slot20() == 0x1a && GetBody()->Slot22())
        GetBody()->Slot21(0x1b, 0, 0);
    if ((state == 0x18 || state == 0) && fabs(c->turn) <= 0.1f)
        return;
    Slot24(2, 0);
    if (g_loopingSound)
        StopLoopingSounds();
}
