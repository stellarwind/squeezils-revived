// 0x47c7b0: identify the ASCII/binary Surface V1.1 header.
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "NavSurface/SurfaceFileReader.h"

extern const char kSurfaceReadMode[];             // 0x4ba008
extern const char kSurfaceOpenError[];            // 0x4c3e54
extern const char kSurfaceHeaderError[];          // 0x4c3e30
extern const char *gSurfaceAsciiHeader;           // 0x4c3e28
extern const char *gSurfaceBinaryHeader;          // 0x4c3e2c

int SurfaceFileReader::DetectFormat(const char *path)
{
    char header[128];
    FILE *file = fopen(path, kSurfaceReadMode);
    if (!file) {
        fprintf(stderr, kSurfaceOpenError, path, strerror(errno));
        return -2;
    }
    if (!fgets(header, sizeof(header), file)) {
        fprintf(stderr, kSurfaceHeaderError, path);
        fclose(file);
        return -2;
    }
    fclose(file);
    if (strcmp(header, gSurfaceAsciiHeader) == 0)
        return 0;
    return strcmp(header, gSurfaceBinaryHeader) == 0 ? 1 : -1;
}
