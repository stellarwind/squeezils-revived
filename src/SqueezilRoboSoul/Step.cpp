// 12 spare declarations (symbol count, see padscan.sh)
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
// SqueezilRoboSoul 0x4045b0 / 0x4045f0: ask the soul's state, tell the soul (slots 41 / 42), then ask the behaviour for the answer.
struct RoboBehavior {
    virtual int Execute(int a, int b);                   // slot 1 is Execute below
};
struct RoboBehavior2 { virtual int s0(int a, int b); virtual int Execute(int a, int b); };
struct StepSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
    virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24();
    virtual int  GetState();                             // slot 25 (+0x64)
    virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29(); virtual void v30(); virtual void v31();
    virtual void v32(); virtual void v33(); virtual void v34(); virtual void v35(); virtual void v36(); virtual void v37();
    virtual void v38(); virtual void v39(); virtual void v40();
    virtual void SetGround(int state, int arg);          // slot 41 (+0xa4)
    virtual void SetAir(int state, int arg);             // slot 42 (+0xa8)
    char pad[0xec - 4];
    RoboBehavior2 *behaviour;                            // 0xec
    void StepGround(int arg, int *out);                  // 0x4045b0
    void StepAir(int arg, int *out);                     // 0x4045f0
};

void StepSoul::StepGround(int arg, int *out)
{
    int s = GetState(); SetGround(s, arg);
    *out = behaviour->Execute(arg, 0);
}

void StepSoul::StepAir(int arg, int *out)
{
    int s = GetState(); SetAir(s, arg);
    *out = behaviour->Execute(0, arg);
}
