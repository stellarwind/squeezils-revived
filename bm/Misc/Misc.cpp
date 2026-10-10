// PrepareForDialog (0x4753c0), SetFloorLevel (0x450c00), SetMaxLandingSpeed (0x450c10).
#include "Misc/Misc.h"

extern float kFloorLevel;        // 0x4c7ac8
extern void *g_defaultPhysics;   // 0x4c7ad0

int PrepareForDialog()
{
    return 0;
}

void SetFloorLevel(float y)
{
    kFloorLevel = y;
}

void SetMaxLandingSpeed(float speed)
{
    *(float *)((char *)g_defaultPhysics + 0x270) = speed;
}

extern void GameLog(const char *);   // 0x419290

void NumericError()
{
    GameLog("number out of range");
}

extern Scene *g_scene;   // 0x4c75e0

void *__stdcall SetDecalSize(void *frame, float width, float height)
{
    return g_scene->SetFrameDecalSize(frame, width, height);
}

extern void GameLog2(const char *format, const char *arg);   // any 2-arg log; target masked

void SqgErrorLog(const char *message)
{
    GameLog2("sqg: %s", message);
}

extern float kSens2;                // 0x4c77a0
extern void ApplySensitivity(float); // 0x45ac40

void SetSensitivity2(float value)
{
    kSens2 = value;
    ApplySensitivity(value);
}
