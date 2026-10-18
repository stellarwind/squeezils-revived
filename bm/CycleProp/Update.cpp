// CycleProp::Update (SQZ.EXE 0x428e90, slot 7, ret 4): FrameProp's update, then step through the children on a timer.
struct CycleU {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void VHide();                       // slot 5 (+0x14)
    char pad[0x44 - 4];
    int count;                                  // 0x44 (the child names)
    char pad1[0xa8 - 0x48];
    int hideAtEnd;                              // 0xa8
    char pad2[0xdc - 0xac];
    int current;                                // 0xdc
    int step;                                   // 0xe0
    int mode;                                   // 0xe4
    int carry;                                  // 0xe8
    float timer;                                // 0xec
    float period;                               // 0xf0
    void UpdateBase(float seconds);             // 0x427740
    void FireStepMessages(int from, int to);    // 0x4290b0
    void SetCurrent(int child);                 // 0x428fd0
    void Update(float seconds);
};

void CycleU::Update(float seconds)
{
    UpdateBase(seconds);
    if (step == 0)
        return;
    timer -= seconds;
    int from = current;
    int next = from;
    if (*(unsigned *)&timer > 0x80000000u) {
        next = from + step;
        do {
            if (carry)
                timer += period;
            else
                timer = period;
        } while (*(unsigned *)&timer > 0x80000000u);
    }
    if (next < count && next >= 0) {
        FireStepMessages(from, next);
    } else if (mode == 1) {
        int below = next < 0;
        FireStepMessages(from, below ? 0 : count - 1);
        next = below ? next + count : next - count;
        FireStepMessages(below ? count - 1 : 0, next);
    } else if (mode == -1) {
        step = -step;
        next = next < 0 ? 0 : count - 1;
        FireStepMessages(from, next);
    } else {
        FireStepMessages(from, next < 0 ? 0 : count - 1);
        if (hideAtEnd)
            VHide();
    }
    SetCurrent(next);
}
