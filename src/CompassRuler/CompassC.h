#pragma once
#include "compat41.h"
#include <windows.h>
struct SurfaceC;
struct StripBitmap {
    virtual int CopyRectFrom(StripBitmap *src, RECT srcRect, RECT dstRect); // +0x00
    virtual void d1(); virtual void d2(); virtual void d3();
    virtual int DrawToSurfaceAt(SurfaceC *dest, int x, int y, int a, int b, int c); // +0x10
    int GetWidth();                              // 0x4739b0
    int GetHeight();                             // 0x4739c0
};
struct CompassC {
    StripBitmap *picture[12];                    // 0x00
    int x, y;                                   // 0x30
    void *sprite[8];                             // 0x38
    int sector, sectorCount;                     // 0x58
    float fullTurn;                             // 0x60
    int pineRegion, desertRegion, coastRegion, jungleRegion; // 0x64
    float quarter, eighth;                       // 0x74
    char name[6][20];                            // 0x7c
    char path[0x48];                             // 0xf4
    int Init();                                 // 0x414490
    void UpdateSpritesReal();                    // 0x414af0
    int Compose(int sector, int other, float fraction, float roll); // 0x4150b0
    void SetCompass(float angle, float roll);    // 0x414fb0
};
