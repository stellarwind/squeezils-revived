// Shared overlay lifetime layout; constructor allocation remains in its caller.
#pragma once
#include "Bitmap2/Bitmap2.h"
struct Vec3 { float x, y, z; Vec3() {} ~Vec3(); };         // 0x401660
struct MemA { char d[0x38c]; MemA(); ~MemA(); };           // 0x42ea70 / 0x42eb40
struct MemB { char d[0x94]; MemB(); ~MemB(); };            // 0x42f580 / 0x42f5f0
struct MemC { char d[0x13c]; MemC(); ~MemC(); void Reset(); };   // 0x414380 / 0x4143c0, Reset 0x414490
struct Overlay {
    int originX, originY;                                  // 0x000
    MemA a;                                                // 0x008
    MemB b;                                                // 0x394
    Bitmap *tiles[10];                                     // 0x428
    int modeA, modeB;                                      // 0x450
    int selected;                                          // 0x458
    int unknown45c;                                        // 0x45c
    Bitmap *main;                                          // 0x460
    char pad464[0x478 - 0x464];
    int unknown478, unknown47c;                            // 0x478
    MemC c;                                                // 0x480
    float step;                                            // 0x5bc
    float verticalThreshold;                               // 0x5c0
    float verticalChange;                                  // 0x5c4
    Vec3 v0, v1, v2, v3, v4, v5;                           // 0x5c8
    float turn;                                            // 0x610
    int unknown614;                                        // 0x614
    HGDIOBJ font;                                          // 0x618
    int lineHeight;                                        // 0x61c
    int messageX, messageY;                                // 0x620
    int maxMessageWidth;                                   // 0x628
    int maxMessageLines;                                   // 0x62c
    char textA[0x694 - 0x630];                             // 0x630
    char textB[1];                                         // 0x694
    Overlay();
    ~Overlay();
    void GetScreenOrigin(int *x, int *y);                  // 0x42da30
    void Layout();                                         // 0x42e7a0
};

inline void DestroyOverlay(void *overlay) { delete (Overlay *)overlay; }
