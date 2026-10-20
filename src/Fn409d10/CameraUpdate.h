// Camera-soul types shared by RegionCameraSoul::Update (0x409f70) and PlaceCameraSoul::Update (0x40a820).
#include "compat41.h"
#include <math.h>

struct CamMatData {                                                // the plain 16 floats (copied inline, element by element)
    float m[16];
    CamMatData &operator=(const CamMatData &o)
    {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
        return *this;
    }
};
struct CamMat {
    CamMatData d;
    CamMat() {}
    CamMat(const CamMat &o);                                       // 0x4099e0
    ~CamMat();                                                     // 0x402310
    CamMat &operator=(const CamMat &o);                            // 0x402340
};
// SIZE 0x60
struct CameraSettings {
    float povOffsetX, povOffsetY, povOffsetZ;                      // 0x00
    int autoLeashMode, lookAtReturnMode;                           // 0x0c
    float leashLength;                                             // 0x14
    char pad18[0x5c - 0x18];
    float returnToBaseTime;                                        // 0x5c
};
extern CameraSettings gRegionSettings;                             // 0x4c64e8
extern float gCamInterpTime;                                       // 0x4c6480 [Camera] CamInterpTime

struct GameClock { unsigned SimRate(); };                          // 0x41ec00: milliseconds this frame
GameClock *TheClock();                                             // 0x4206d0

struct CamBodyC {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual CamMatData *GetMatrix();                               // vtable+0x0c
    char pad4[0xc0 - 4];
    CamMatData lastMatrix;                                         // 0xc0 the matrix of the previous frame
};
struct CamViewC {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetMatrix(CamMat *m);                             // vtable+0x24
};

struct CamSoulC { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11(); virtual void v12();
    virtual void v13(); virtual void v14(); virtual void v15();
    virtual void Unfreeze();                                       // vtable+0x40
};

// SIZE 0x168
struct CameraSoulU {
    void *vtbl;                                                    // 0x00
    void *inputVtbl;                                               // 0x04
    CamViewC *view;                                                // 0x08
    CamBodyC *body;                                                // 0x0c
    CamSoulC *targetSoul;                                          // 0x10
    int enabled;                                                   // 0x14 PlaceCameraSoul: 0 while a path plays
    int pad18[8];
    float lookPitch;                                               // 0x38
    float lookYaw;                                                 // 0x3c
    float returnPitchRate;                                         // 0x40
    float returnYawRate;                                           // 0x44
    CamMatData unused48;                                           // 0x48
    CamMatData fromMatrix;                                         // 0x88
    CamMatData unusedC8;                                           // 0xc8
    CamMat toMatrix;                                               // 0x108
    int lookMode;                                                  // 0x148 0 free, 1 just released, 2 returning
    float blendTime;                                               // 0x14c
    float blendDuration;                                           // 0x150
    int blending;                                                  // 0x154
    int blendPosition;                                             // 0x158
    int justStarted;                                               // 0x15c
    struct CamPathC *path;                                         // 0x160
    int pathMode;                                                  // 0x164 1 world, 2 relative to the player's place, else to its matrix
    CamMat Compute(CameraSettings *s, CamMat m);                   // 0x408850
    CamMat BlendMatrix();                                          // 0x409620
};

#define BITS(f) (*(int *)&(f))
