// SqueezilRoboSoul::PickState (0x404840): flat layout, virtual calls by slot.
struct RBody {
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
    virtual int IsOnGround();
};
struct RoboSoulFlat {
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
    virtual RBody *Body();
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void s22();
    virtual void s23();
    virtual void s24();
    virtual int GetState();
    char pad0[0x84 - 4];
    int wasOnGround;           // +0x84
    char pad1[0xec - 0x88];
    void *behaviour;           // +0xec
    int PickState();
};

int RoboSoulFlat::PickState()
{
    int state = GetState();
    int onGround = Body()->IsOnGround();
    if (!behaviour) {
        state = 0;
        wasOnGround = Body()->IsOnGround();
    } else {
        if (state == 0x15 || (state == 0x16 && onGround))
            state = 3;
        else if (state == 0x14 || (state == 0x16 && !onGround))
            state = 4;
        else if (wasOnGround && !onGround) {
            wasOnGround = 0;
            state = 1;
        } else if (!wasOnGround && onGround) {
            wasOnGround = 1;
            state = 2;
        } else
            state = 0x13;
    }
    return state;
}
