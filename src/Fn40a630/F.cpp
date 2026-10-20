// PlaceCameraSoul constructor (SQZ.EXE 0x40a630, ret 0xc): the inline base chain, then CameraSoul's own body.
#include "compat41.h"
struct MMap { virtual ~MMap(); };                                  // vtable 0x49d2b4
struct Soul : MMap { virtual ~Soul(); };                           // 0x49d2a4
struct SimulationSoul : Soul { virtual ~SimulationSoul(); };       // 0x49d2ec
struct EventTarget { virtual void Event(); };                       // vtable 0x49d2c4
struct InputTarget : EventTarget { virtual void Input(); };                       // second base (vtable 0x49d314)
struct Six { int w[6]; };
struct CamVec { float x, y, z; CamVec() {} ~CamVec(); };           // dtor 0x401660
struct CamMat {
    float m[16];
    CamMat() {}
    CamMat(const CamMat &o);                                       // 0x4099e0
    ~CamMat();                                                     // 0x402310
};
struct CameraSettings {
    float povOffsetX, povOffsetY, povOffsetZ;
    int autoLeashMode, lookAtReturnMode;
    float leashLength;
    char rest[0x5c - 0x18];
};
void __stdcall ReadCameraSettings(const char *section, CameraSettings *out);   // 0x408600
extern const char kPlaceCamSection[];                                   // 0x4b9ab8
struct CamBodyC {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual CamMat *GetMatrix();                                   // slot 3 (+0xc)
};
struct CamViewC {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetMatrix(CamMat *m);                             // slot 9 (+0x24)
};
struct CameraSoulC : SimulationSoul, InputTarget {
    CamViewC *view;                                                // 0x08
    CamBodyC *body;                                                // 0x0c
    int c10;                                                       // 0x10
    int on14;
    CamVec focus;                                                  // 0x18
    int a24, a28, a2c;                                           // 0x24
    float scale30;                                                 // 0x30
    int pad34;
    int a38;                                                       // 0x38
    int a3c;                                                       // 0x3c
    int pad40[2];
    CamMat mat0, mat1, mat2, mat3;                                 // 0x48 0x88 0xc8 0x108
    int blend148;
    int pad14c, pad150;
    int started;                                                   // 0x154
    int pad158;
    int flag15c;                                                   // 0x15c
    CamMat Compute(CameraSettings *s, CamMat m);                   // 0x408850
    virtual ~CameraSoulC();
};
struct PlaceCameraSoulC : CameraSoulC {
    PlaceCameraSoulC(CamViewC *v, CamBodyC *b, int c);
    virtual void Input();
};

PlaceCameraSoulC::PlaceCameraSoulC(CamViewC *v, CamBodyC *b, int c)
{
    view = v;
    body = b;
    c10 = c;
    on14 = 1;
    static CameraSettings settings;
    static int done;
    if (!done) {
        ReadCameraSettings(kPlaceCamSection, &settings);
        done = 1;
    }
    float px = settings.povOffsetX, pz = settings.povOffsetZ, py = settings.povOffsetY;
    focus.x = px;
    focus.y = py;
    focus.z = pz;
    a24 = settings.autoLeashMode;
    a28 = settings.lookAtReturnMode;
    a2c = *(int *)&settings.leashLength;
    a38 = 0;
    scale30 = 1.0f;
    a3c = 0;
    CamMat r = Compute(&settings, *body->GetMatrix());
    view->SetMatrix(&r);
    started = 0;
    blend148 = 0;
    flag15c = 1;
}
