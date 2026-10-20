// SQZ.EXE 0x457050: initialize body settings, scene frame and animated character.
#include "IniReader/IniReader.h"

struct CtorBodyBase {
    void *vtable;
    float matrix[16];
    char pad44[0x6c - 0x44];
    float mass;
    char pad70[0xb8 - 0x70];
    CtorBodyBase(const float *matrix); // 0x41fb90
    ~CtorBodyBase(); // 0x40bae0
};
struct CtorBodyMatrix {
    float m[16];
    CtorBodyMatrix() {}
    ~CtorBodyMatrix(); // 0x402310
};
struct CtorAnimatedBody {
    virtual void slot0();
    virtual void Build(void *frame); // +4
    virtual void slot2();
    virtual void SetCycle(int clip, int blend); // +0xc
    char data[0x50 - 4];
    CtorAnimatedBody(CtorAnimatedBody *source); // 0x41dab0
};
struct CtorBodyScene {
    void *CreateSceneFrame(); // 0x476f40
};
extern CtorBodyScene *gCtorBodyScene; // 0x4c75e0
extern void *kSqueezilBodyVtable[]; // 0x49fe4c
extern int gBodySettingsRead; // 0x4bf068
extern float gMorphTimeScale; // 0x4c7b84
extern float gNonFlyMorphRotation; // 0x4c7b90
extern float gMorphDisableDistance; // 0x4c7bac
extern float gDefaultSqueezilMass; // 0x4c7ba8
extern float gFlyBank; // 0x4c7b8c
extern float gClimbAngle; // 0x4c7b80
extern float gDiveAngle; // 0x4c7b7c
extern float gBlendSeconds; // 0x4c7b88
extern CtorAnimatedBody *gProtoBodies[]; // 0x4bf138
extern int gClips[]; // 0x4bf070

struct ConstructingSqueezilBody : CtorBodyBase {
    float collisionRadius, collisionSpeedAmp; // +0xb8
    CtorBodyMatrix lastMatrix, shownMatrix, blendFrom, unused180, blendTo; // +0xc0
    float blendTime; // +0x200
    int blending, noBlend, unknown20c;
    float orientation[3], clipSpeed;
    void *frame; // +0x220
    int onGround, unknown228;
    CtorAnimatedBody *character;
    int mode, previousMode;
    ConstructingSqueezilBody(const float *matrix, int color);
};

ConstructingSqueezilBody::ConstructingSqueezilBody(const float *m, int color)
    : CtorBodyBase(m)
{
    vtable = kSqueezilBodyVtable;
    IniReader ini;
    if (!gBodySettingsRead) {
        gMorphTimeScale = ini.GetFloat("MorphTimeScale", 2.0f);
        gNonFlyMorphRotation = ini.GetFloat("NonFlyMorphRotation", -90.0f);
        gMorphDisableDistance = ini.GetFloat("MorphDisableDistance", 15.0f);
        ini.SetSection("Physics");
        gDefaultSqueezilMass = ini.GetFloat("DefaultSqueezilMass", 0.015625f);
        gFlyBank = ini.GetFloat("FlyBank", 60.0f);
        gClimbAngle = ini.GetFloat("ClimbAngle", -45.0f);
        gDiveAngle = ini.GetFloat("DiveAngle", 45.0f);
        ini.SetSection("Camera");
        gBlendSeconds = ini.GetFloat("CamInterpTime", 0.5f);
        gBodySettingsRead = 1;
    }
    ini.SetSection("Physics");
    collisionRadius = ini.GetFloat("CollisionRadius", 0.5f);
    collisionSpeedAmp = ini.GetFloat("CollisionSpeedAmp", 1.0f);
    mass = gDefaultSqueezilMass;
    matrix[0]=1.0f; matrix[1]=0; matrix[2]=0; matrix[3]=0;
    matrix[4]=0; matrix[5]=1.0f; matrix[6]=0; matrix[7]=0;
    matrix[8]=0; matrix[9]=0; matrix[10]=1.0f; matrix[11]=0;
    matrix[12]=0; matrix[13]=0; matrix[14]=0; matrix[15]=1.0f;
    clipSpeed=0;
    orientation[2]=0; orientation[1]=0; orientation[0]=0;
    frame = gCtorBodyScene->CreateSceneFrame();
    mode = 0;
    previousMode = 0;
    character = new CtorAnimatedBody(gProtoBodies[color]);
    character->Build(frame);
    character->SetCycle(gClips[mode], 0);
    unknown228=1;
    blending=0;
    blendTime=0;
    noBlend=1;
    unknown20c=0;
}
