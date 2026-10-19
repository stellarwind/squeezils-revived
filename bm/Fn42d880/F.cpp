// 0x42d880: destructor of the display/overlay object: deletes its bitmaps, releases a GDI handle, destroys the members.
#include <windows.h>
#include "Bitmap2/Bitmap2.h"
struct Vec3 { float x, y, z; Vec3() {} ~Vec3(); };         // 0x401660
struct MemA { char d[0x38c]; ~MemA(); };                   // 0x42eb40
struct MemB { char d[0x94]; ~MemB(); };                    // 0x42f5f0
struct MemC { char d[0x148]; ~MemC(); };                   // 0x4143c0
struct Overlay {
    char pad0[8];
    MemA a;                                                // 0x008
    MemB b;                                                // 0x394
    Bitmap *tiles[10];                                     // 0x428
    char pad450[0x460 - 0x450];
    Bitmap *main;                                          // 0x460
    char pad464[0x480 - 0x464];
    MemC c;                                                // 0x480
    Vec3 v0, v1, v2, v3, v4, v5;                           // 0x5c8
    char pad610[0x618 - 0x610];
    HGDIOBJ font;                                          // 0x618
    ~Overlay();
};
Overlay::~Overlay()
{
    delete main;
    for (int i = 0; i < 10; i++)
        delete tiles[i];
    if (font)
        DeleteObject(font);
}
