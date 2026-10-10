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
