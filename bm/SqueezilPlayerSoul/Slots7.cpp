// SqueezilPlayerSoul slot at SQZ.EXE 0x454ab0 (ret 8): count the timer down by the frame time; when it runs out go to state 0x11, else fill the control from the physics block.
struct Motion { int w[0x17]; };
struct Ctl { float forward, turn; Motion motion; };
struct Physics { char pad[0x13c]; Motion block; };
class Clock { public: unsigned Milliseconds(); };         // 0x41ec00
Clock *GetClock();                                       // 0x4206d0 (cdecl)
struct Soul7 {
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
    virtual void Slot24(int a, int b);
    virtual void s25();
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
    virtual void s44();
    virtual void Slot45(struct Ctl *c, int arg);
    char pad[0x48 - 4 - 4 * 0];
    Physics *physics;                                    // 0x48
    char pad2[0x78 - 0x4c];
    float timer;                                         // 0x78
    void Run(Ctl *c, int arg);
};

void Soul7::Run(Ctl *c, int arg)
{
    float dt = (float)GetClock()->Milliseconds() * 0.001f; timer = timer - dt;
    if (*(int *)&timer <= 0) {
        Slot24(0x11, 0);
        return;
    }
    c->motion = physics->block;
    c->forward = 0.5f;
    c->turn = 0.0f;
    Slot45(c, arg);
}
