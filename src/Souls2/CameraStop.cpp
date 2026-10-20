// CameraSoul::StopPath (0x409c40): flat layout, the camera's virtual GetMatrix is slot 3.
struct CamBody {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual int *GetMatrix();
};

struct CameraSoulFlat {
    char pad0[8];
    CamBody *camera;           // +8
    char pad1[0x14 - 0xc];
    int enabled;               // +0x14
    char pad2[0x88 - 0x18];
    int fromMatrix[16];        // +0x88
    char pad3[0x148 - 0xc8];
    int lookMode;              // +0x148
    float blendTime;           // +0x14c
    float blendDuration;       // +0x150
    int blending;              // +0x154
    int blendPosition;         // +0x158
    void StopPath(float duration);
};

void CameraSoulFlat::StopPath(float duration)
{
    enabled = 1;
    blendTime = 0.0f;
    blendDuration = duration;
    int *m = camera->GetMatrix();
    fromMatrix[0] = m[0]; fromMatrix[1] = m[1]; fromMatrix[2] = m[2]; fromMatrix[3] = m[3];
    fromMatrix[4] = m[4]; fromMatrix[5] = m[5]; fromMatrix[6] = m[6]; fromMatrix[7] = m[7];
    fromMatrix[8] = m[8]; fromMatrix[9] = m[9]; fromMatrix[10] = m[10]; fromMatrix[11] = m[11];
    fromMatrix[12] = m[12]; fromMatrix[13] = m[13]; fromMatrix[14] = m[14]; fromMatrix[15] = m[15];
    blending = 1;
    blendPosition = 1;
    lookMode = 1;
}
