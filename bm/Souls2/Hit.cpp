// SqueezilSoul::Hit (0x4506d0) and DirectionHistory::Clear (0x456ef0).
// two spare declarations: the compiler's register choices follow the symbol count (tools/flagsearch/padscan.sh)
void PadDecl1();
void PadDecl2();
struct HBody {
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
    virtual int IsOnGround();   // 15, +0x3c
};
struct HPhys { char pad[0x274]; int tumbleDuration; };
struct HitSoul {
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
    virtual void SprayNuts();   // 12, +0x30
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual HBody *Body();      // 17, +0x44
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void s22();
    virtual void s23();
    virtual void SetState(int s, int blend);   // 24, +0x60
    int pad0;
    char pad1[0xc - 8];
    int spitPhase;            // +0xc
    char pad2[0x3c - 0x10];
    int state;                // +0x3c
    char pad3[0x48 - 0x40];
    HPhys *physics;           // +0x48
    char pad4[0x78 - 0x4c];
    int tumbleTime;           // +0x78
    void Hit(int knock, int sprayNuts, int unused);
};

void HitSoul::Hit(int knock, int sprayNuts, int unused)
{
    if (knock) {
        if (!Body()->IsOnGround()) {
            if (state != 20) {
                SetState(11, 0);
                tumbleTime = physics->tumbleDuration;
            }
        } else if (state != 21)
            SetState(8, 0);
    }
    if (sprayNuts)
        SprayNuts();
    spitPhase = 0;
}

struct DV {
    float x, y, z;
    DV &operator=(const DV &o) { x = o.x; y = o.y; z = o.z; return *this; }
};
struct DirHist {
    int size;
    DV *slots;
    DV sum;
    int next;
    int added;
    void Clear();
};

void DirHist::Clear()
{
    sum.x = 0.0f;
    sum.y = 0.0f;
    sum.z = 0.0f;
    for (int i = 0; i < size; i++) {
        slots[i] = sum;
    }
    next = 0;
    added = 0;
}
