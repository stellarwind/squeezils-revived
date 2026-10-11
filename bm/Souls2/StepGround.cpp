// MoveActivity::StepGround (0x401430): flat layout, virtual calls by slot.
struct ABody {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual float Speed();      // slot 5, +0x14
};
struct ASoul {
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
    virtual void s24();
    virtual int GetState();     // slot 25, +0x64
};
// (one more declaration ahead of the code, like the original header would have; the compiler's tables make that matter)
void MoveActivityPad();

struct GroundControl {
    float forward;
    float turn;
};

extern float kStillSpeed;     // 0x49d014

struct MoveActivityFlat {
    int vtbl;                    // +0
    ABody *body;                 // +4
    ASoul *soul;                 // +8
    char pad[0x2ac - 0xc];
    int direction;               // +0x2ac
    int turn;                    // +0x2b0
    float turnAmount;            // +0x2b4
    int gait;                    // +0x2b8
    int StepGround(GroundControl *out);
};

int MoveActivityFlat::StepGround(GroundControl *out)
{
    int state = soul->GetState();
    int wanted = 24;
    switch (gait) {
    case 0:
        out->forward = 0.0f;
        if (state != 0 && body->Speed() <= kStillSpeed)
            wanted = 0;
        break;
    case 3:
        out->forward = (float)direction;
        if (state != 3 && body->Speed() > kStillSpeed)
            wanted = 3;
        break;
    case 4:
    case 7:
        out->forward = 1.0f;
        if (state != gait)
            wanted = gait;
        break;
    default:
        break;
    }
    out->turn = (float)turn * turnAmount;
    return wanted;
}
