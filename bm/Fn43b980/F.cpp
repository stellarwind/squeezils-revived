// 0x43b980: switches a mode on/off (ret 4); callers 4.
struct PartA { void A(int); void B(int); };       // 0x4592a0, 0x4592f0
struct PartB { void C(int); };                    // 0x4479d0
struct Ctl { void On(); void Off(); };            // 0x43c1b0, 0x43c210
struct Ctx { int Busy(); };                       // 0x44eea0
Ctl *GetCtl();                                    // 0x4206b0
extern Ctx *g_ctx;                                // 0x4c75e8
struct Obj {
    char pad0[8];
    PartA *a;           // 0x08
    char pad1[4];
    PartB *b;           // 0x10
    char pad2[0x78 - 0x14];
    int active;         // 0x78
    char pad3[0x98 - 0x7c];
    int mode;           // 0x98
    int Test();         // 0x43c7a0
    void Refresh();     // 0x43ba40
    void SetActive(int on);
};
void Obj::SetActive(int on)
{
    if (on) {
        if (mode) a->A(0);
        a->B(1);
    } else {
        if (mode == 1) {
            a->A(1);
            a->B(1);
        }
        if (mode == 2)
            b->C(1);
        else if (Test())
            b->C(0);
        Refresh();
    }
    active = on;
    if (on) {
        if (!g_ctx->Busy())
            GetCtl()->On();
    } else {
        if (!g_ctx->Busy())
            GetCtl()->Off();
    }
}
