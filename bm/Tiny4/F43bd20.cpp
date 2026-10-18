// 0x43bd20: current input value of a control, from whichever source (timed ramp, joystick, mouse) its mode selects.
struct Mouse459 { float Axis(); void M4591b0(int); };          // 0x459450, 0x4591b0
struct Stick447 { int Active(); };                              // 0x4479c0
struct Control43bd20 {
    char pad[8];
    Mouse459 *mouse;            // 0x08
    char pad2[4];
    Stick447 *stick;            // 0x10
    char pad3[0x50 - 0x14 - 0x20 + 0x20];
    int timed;                  // 0x50
    char pad4[0x74 - 0x54];
    int disabled;               // 0x74
    char pad5[0x84 - 0x78];
    int reset;                  // 0x84
    char pad6[0x98 - 0x88];
    int mode;                   // 0x98
    float Ramp();               // 0x43bf80
    float Stickish();           // 0x43bfe0
    float Value();
};
float Control43bd20::Value()
{
    float r;
    if (disabled)
        return 0.0f;
    if (!timed) {
        r = 0.0f;
        if (mode == 2 && stick->Active())
            r = Stickish();
        else if (mode == 1) {
            r = mouse->Axis();
            if (reset == 1)
                mouse->M4591b0(0);
        }
        return r;
    }
    r = Ramp();
    return r;
}
