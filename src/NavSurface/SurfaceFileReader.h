// Surface-file input and the point/radius fields touched by its loader.
#pragma once
#include <stdio.h>

struct SurfaceFilePoint { float x, y, z; };
struct SurfaceFileReader {
    unsigned pointCount;
    SurfaceFilePoint *points;
    char pad08[0x2020 - 8];
    float radiusSquared;
    int DetectFormat(const char *path); // 0x47c7b0
    int Load(const char *path);         // 0x47c8e0
    int ReadAscii(FILE *file);          // 0x47ca10
    int ReadBinary(FILE *file);         // 0x47cfe0
};
