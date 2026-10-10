// Misc free functions (SQZ.EXE). PrepareForDialog does nothing and returns 0.
#include "compat41.h"

int PrepareForDialog();   // 0x4753c0
void SetFloorLevel(float y);          // 0x450c00
void SetMaxLandingSpeed(float speed); // 0x450c10
void NumericError();                  // 0x41eb80

struct Scene {
    void *SetFrameDecalSize(void *frame, float width, float height);  // 0x477540
};
void *__stdcall SetDecalSize(void *frame, float width, float height); // 0x435c70 (forwards to the global scene)
void  SqgErrorLog(const char *message);          // 0x4241b0
void  SetSensitivity2(float value);              // 0x44aab0

struct ClientBitmap {
    int f0;    // 0x00
    int f4;    // 0x04
    int f8;    // 0x08
    int fc;    // 0x0c
    int f10;   // 0x10
    int f14;   // 0x14
    ClientBitmap *Init();  // 0x478150
};
