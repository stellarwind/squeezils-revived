// 0x47c8e0: load a Surface file and update the enclosing radius.
// 10 spare declarations (register allocation, see padscan.sh).
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
#include <string.h>
#include <errno.h>
#include "NavSurface/SurfaceFileReader.h"

extern const char kSurfaceReadMode[];     // 0x4ba008
extern const char kSurfaceBinaryMode[];   // 0x4baa40
extern const char kSurfaceOpenError[];    // 0x4c3e54
extern const char kSurfaceInvalidHeader[]; // 0x4c3e70

int NavMesh::Load(const char *path)
{
    int format = DetectFormat(path);
    switch (format) {
    case -2: return 0;
    case -1:
        fprintf(stderr, kSurfaceInvalidHeader, path);
        return 0;
    }
    FILE *file = 0;
    switch (format) {
    case 0: file = fopen(path, kSurfaceReadMode); break;
    case 1: file = fopen(path, kSurfaceBinaryMode); break;
    }
    if (!file) {
        fprintf(stderr, kSurfaceOpenError, path, strerror(errno));
        return 0;
    }
    int result;
    switch (format) {
    case 0: ReadAscii(file); // Preserve the original fallthrough.
    case 1: result = ReadBinary(file); break;
    }
    unsigned remaining = pointCount;
    if (remaining) {
        float *point = &points->y;
        do {
            float squared = point[-1] * point[-1] + point[0] * point[0] + point[1] * point[1];
            if (squared > radius)
                radius = squared;
            point += 3;
        } while (--remaining);
    }
    fclose(file);
    return result;
}
