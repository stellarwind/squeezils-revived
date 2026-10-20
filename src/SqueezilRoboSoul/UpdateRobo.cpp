// SqueezilRoboSoul::UpdateRobo (SQZ.EXE 0x404df0, slot 5): one frame: the state machine until a state is taken, then its function runs.
struct BodyU { virtual void b0(); virtual void b1(); virtual void b2(); virtual void b3(); virtual void b4(); virtual void b5();
               virtual void b6(); virtual void b7(); virtual void b8(); virtual void b9(); virtual void b10(); virtual void b11();
               virtual void b12(); virtual void b13(); virtual void b14(); virtual int IsOnGround(); };   // slot 15 (+0x3c)
struct TemplateU {
    int TrySetState(int state, struct RoboU *soul);    // 0x4486f0
    void RunPending(struct RoboU *soul);               // 0x4487f0
};
struct RoboU {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16();
    virtual BodyU *GetBody();                          // slot 17 (+0x44)
    virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void s24(); virtual void s25(); virtual void s26(); virtual void s27(); virtual void s28(); virtual void s29();
    virtual void s30(); virtual void s31(); virtual void s32(); virtual void s33(); virtual void s34(); virtual void s35();
    virtual void s36(); virtual void s37(); virtual void s38(); virtual void s39(); virtual void s40(); virtual void s41();
    virtual void s42(); virtual void s43(); virtual void s44(); virtual void s45(); virtual void s46(); virtual void s47();
    virtual void s48(); virtual void s49(); virtual void s50(); virtual void s51(); virtual void s52(); virtual void s53();
    virtual void s54(); virtual void s55(); virtual void s56(); virtual void s57(); virtual void s58(); virtual void s59();
    virtual void SetBehaviour(int mode, const float *args);   // slot 60 (+0xf0)
    char pad[0x80 - 4];
    int step;                                          // 0x80
    char pad2[0x94 - 0x84];
    int retarget;                                      // 0x94
    char pad3[0xa0 - 0x98];
    int lastSenseKind;                                 // 0xa0
    struct Rec { int id; char rest[0x14]; } records[3];   // 0xa4
    char pad4[0xf0 - 0xec];
    int mode;                                          // 0xf0
    TemplateU *character;                              // 0xf4
    int StepAI();                                      // 0x404a80
    int TargetGone(int kind);                          // 0x4047c0
    void BaseUpdate();                                 // 0x450e90
    void UpdateRobo();
};

void RoboU::UpdateRobo()
{
    for (;;) {
        int wanted = StepAI();
        if (wanted == 0x13)
            break;
        if (character->TrySetState(wanted, this))
            break;
    }
    step = 0;
    retarget = 0;
    character->RunPending(this);
    int *pk = &lastSenseKind;

    if (mode == 3 || mode == 4 || mode == 5) {
        if (*pk == 0x3e7 || TargetGone(*pk) || records[*pk].id == -1) {
            int onGround = GetBody()->IsOnGround();
            if (!character->TrySetState(onGround ? 0xb : 0xc, this))
                character->TrySetState(0, this);
            character->RunPending(this);
            if (mode == 3 || mode == 4 || mode == 5) {
                float args[3] = { 90.0f, 5.0f, 1.0f };
                SetBehaviour(0, args);
            }
        }
    }
    BaseUpdate();
}
