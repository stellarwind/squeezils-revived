// Test slots of the simple If conditions (SQZ.EXE 0x443800-0x443de9).
#include "FunctionIf/IfClasses.h"

struct TestCall {                        // ScriptCall seen through its accessors
    void *Actor();                       // 0x420c20
    int Other();                         // 0x420c60
};
struct TestBody {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual int OnGround();              // slot 15 (0x3c)
};
struct TestActor {
    int pad4;
    int dead;                            // 0x08
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16();
    virtual TestBody *Body();            // slot 17 (0x44)
    virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21(); virtual void s22();
    virtual void s23(); virtual void s24();
    virtual int State();                 // slot 25 (0x64)
};

int IfOnTreeFunction::Test(ScriptCall *args, int extra)
{
    return ((TestActor *)((TestCall *)args)->Actor())->Body()->OnGround();
}

int IfPlayerDeadFunction::Test(ScriptCall *args, int extra)
{
    return ((TestActor *)((TestCall *)args)->Actor())->dead;
}

int IfPlayerFunction::Test(ScriptCall *args, int extra)
{
    return !((TestCall *)args)->Other();
}

int IfDyingOrDeadFunction::Test(ScriptCall *args, int extra)
{
    int s = ((TestActor *)((TestCall *)args)->Actor())->State();
    return s == 0x15 || s == 0x14 || s == 0x16;
}
