// Pad::GetAxes (SQZ.EXE 0x43bdc0, ret 8): read both analogue axes from the joystick, else from the keys, else 0.
struct PadStick {
    float AxisY();              // 0x4593d0
    float AxisX();              // 0x459450
    void Center(int mode);      // 0x4591b0
};
struct PadDevice { int Ready(); };   // 0x4479c0

struct PadG {
    char pad0[8];
    PadStick *stick;            // 0x08
    int pad0c; PadDevice *device;  // 0x10
    char pad1[0x3c - 0x14];
    int keysY;                  // 0x3c
    char pad2[0x50 - 0x40];
    int keysX;                  // 0x50
    char pad3[0x74 - 0x54];
    int enabled;                // 0x74
    char pad4[0x84 - 0x78];
    int centerMode;             // 0x84
    char pad5[0x98 - 0x88];
    int source;                 // 0x98
    float KeyY();               // 0x43c0c0
    float KeyYAlt();            // 0x43bf10
    float KeyX();               // 0x43bfe0
    float KeyXAlt();            // 0x43bf80
    void GetAxes(float *y, float *x);
};

void PadG::GetAxes(float *y, float *x)
{
    if (!enabled) {
        *y = 0;
        *x = 0;
        return;
    }
    *x = -2.0f;
    *y = -2.0f;
    if (!keysY) {
        if (source == 2 && device->Ready())
            *y = KeyY();
        else if (source == 1) {
            *y = stick->AxisY();
            if (centerMode == 1)
                stick->Center(1);
        }
    }
    if (*y == -2.0f) {
        if (!keysY)
            *y = 0;
        else
            *y = KeyYAlt();
        if (*y == -2.0f)
            *y = 0;
    }
    if (!keysX) {
        if (source == 2 && device->Ready())
            *x = KeyX();
        else if (source == 1) {
            *x = stick->AxisX();
            if (centerMode == 1)
                stick->Center(0);
        }
    }
    if (*x == -2.0f) {
        if (!keysX)
            *x = 0;
        else
            *x = KeyXAlt();
        if (*x == -2.0f)
            *x = 0;
    }
}
