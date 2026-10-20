// Print/dump methods of the script-function classes (SQZ.EXE 0x421a70-0x470400): log the class name and its operands.
void DebugLog(const char *fmt, ...);   // 0x419290

struct Text { int v; const char *Get(); };     // 0x438530 (thiscall)
struct Child {
    virtual void M0(); virtual void M1(); virtual void M2(); virtual void M3(); virtual void M4();
    virtual void M5(); virtual void M6();
    virtual void M7();
};
struct Node { int Idx(int); };          // 0x421150

struct T {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual const char *Name();            // slot 4
    union { Child *a; Node *n4; Text f4; };
    union { Child *b; Node *n8; Text f8; };
    Text f12;
    void f_423e00();
    void f_4406a0();
    void f_4407f0();
    void f_442380();
    void f_442630();
    void f_444740();
    void f_4237d0();
    void f_423c70();
    void f_441cc0();
    void f_444930();
    void f_44e760();
    void f_423a10();
    void f_4409f0();
    void f_441800();
    void f_444e90();
    void f_441a60();
    void f_444040();
    void f_421a70();
    void f_440060();
    void f_440330();
    void f_4404f0();
    void f_4424c0();
    void f_444300();
    void f_444550();
};

void T::f_423e00()
{
    DebugLog("%s", Name());
}

void T::f_4406a0()
{
    DebugLog("%s", Name());
}

void T::f_4407f0()
{
    DebugLog("%s", Name());
}

void T::f_442380()
{
    DebugLog("%s", Name());
}

void T::f_442630()
{
    DebugLog("%s", Name());
}

void T::f_444740()
{
    DebugLog("%s", Name()); DebugLog("\n");
}

void T::f_4237d0()
{
    DebugLog("%s", Name()); a->M0(); DebugLog("\n");
}

void T::f_423c70()
{
    DebugLog("%s", Name()); a->M0(); DebugLog("\n");
}

void T::f_441cc0()
{
    DebugLog("%s", Name()); a->M0(); DebugLog("\n");
}

void T::f_444930()
{
    DebugLog("%s", Name()); a->M0(); DebugLog("\n");
}

void T::f_44e760()
{
    DebugLog("%s", Name()); a->M0(); DebugLog("\n");
}

void T::f_423a10()
{
    DebugLog("%s", Name()); a->M0(); b->M0(); DebugLog("\n");
}

void T::f_4409f0()
{
    DebugLog("%s", Name()); a->M0(); b->M0(); DebugLog("\n");
}

void T::f_441800()
{
    DebugLog("%s", Name()); a->M0(); b->M0(); DebugLog("\n");
}

void T::f_444e90()
{
    DebugLog("%s", Name()); a->M0(); b->M0(); DebugLog("\n");
}

void T::f_441a60()
{
    DebugLog("%s", Name()); a->M0(); DebugLog("%s", f8.Get());
}

void T::f_444040()
{
    DebugLog("%s", f8.Get());
}

void T::f_421a70()
{
    DebugLog("%s", f12.Get());
}

void T::f_440060()
{
    DebugLog("%s %s", Name(), f4.Get());
}

void T::f_440330()
{
    DebugLog("%s %s", Name(), f4.Get());
}

void T::f_4404f0()
{
    DebugLog("%s %s", Name(), f4.Get());
}

void T::f_4424c0()
{
    DebugLog("%s %d", Name(), n4->Idx(0));
}

void T::f_444300()
{
    const char *s = f4.Get(); DebugLog("%s %s %d", Name(), s, n8->Idx(0)); DebugLog("\n");
}

void T::f_444550()
{
    const char *s = f4.Get(); DebugLog("%s %s %d", Name(), s, n8->Idx(0)); DebugLog("\n");
}

