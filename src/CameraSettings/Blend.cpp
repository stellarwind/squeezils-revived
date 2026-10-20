// CameraSoul::StartBlend (SQZ.EXE 0x409c40): remember the camera's current matrix as the start of a blend of `duration` seconds.
struct M16 {
    float m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15;
    M16 &operator=(const M16 &o)
    {
        m0 = o.m0; m1 = o.m1; m2 = o.m2; m3 = o.m3; m4 = o.m4; m5 = o.m5; m6 = o.m6; m7 = o.m7;
        m8 = o.m8; m9 = o.m9; m10 = o.m10; m11 = o.m11; m12 = o.m12; m13 = o.m13; m14 = o.m14; m15 = o.m15;
        return *this;
    }
};
struct CamBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual M16 *GetMatrix();                                      // slot 3 (+0xc)
};
struct BlendCam {
    char pad0[0x8];
    CamBody *camera;                                               // 0x08
    char pad1[0x14 - 0xc];
    int active;                                                    // 0x14
    char pad2[0x88 - 0x18];
    M16 fromMatrix;                                                // 0x88
    char pad3[0x148 - 0xc8];
    int blending;                                                  // 0x148
    float elapsed;                                                 // 0x14c
    float duration;                                                // 0x150
    int started;                                                   // 0x154
    int flag158;                                                   // 0x158
    void StartBlend(float seconds);                                // 0x409c40
};
void BlendCam::StartBlend(float seconds)
{
    active = 1;
    elapsed = 0.0f;
    duration = seconds;
    fromMatrix = *camera->GetMatrix();
    started = 1;
    flag158 = 1;
    blending = 1;
}
