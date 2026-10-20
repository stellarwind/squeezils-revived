// 0x43bc70: sibling of Control43bd20::Value for the other input channel.
struct Mouse459 { float Axis2(); void M4591b0(int); };         // 0x4593d0, 0x4591b0
struct Stick447 { int Active(); };                              // 0x4479c0
struct Control43bc70 {
    char pad[8];
    Mouse459 *mouse;            // 0x08
    char pad2[4];
    Stick447 *stick;            // 0x10
    char pad3[0x3c - 0x14];
    int timed;                  // 0x3c
    char pad4[0x74 - 0x40];
    int disabled;               // 0x74
    char pad5[0x80 - 0x78];
    int mouseOn;                // 0x80
    int reset;                  // 0x84
    char pad6[0x98 - 0x88];
    int mode;                   // 0x98
    float Ramp();               // 0x43bf10
    float Stickish();           // 0x43c0c0
    float Value();
};
float Control43bc70::Value()
{
    float r;
    if (disabled)
        return 0.0f;
    if (!timed) {
        r = 0.0f;
        if (mode == 2 && stick->Active())
            r = Stickish();
        else if (mode == 1 && mouseOn) {
            r = mouse->Axis2();
            if (reset == 1)
                mouse->M4591b0(1);
        }
        return r;
    }
    r = Ramp();
    return r;
}
