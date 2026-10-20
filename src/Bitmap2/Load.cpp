// Bitmap::Load (0x4739d0): loads an 8 bit Windows bitmap into a new surface in the display's pixel format.
#include <stdlib.h>
#include "Bitmap2/Bitmap2.h"

void ImageConvertRow(void *dst, RenderFmt *fmt, const unsigned char *src, const unsigned char *palette, int width);

#define READ_RETRY(BUF, COUNT, CLEANUP)                                                             \
    rc = 0;                                                                                         \
    answer = 1;                                                                                     \
    while (rc == 0 || answer == 4) {                                                                \
        rc = ReadFile(file, BUF, COUNT, &got, 0);                                                   \
        if (rc == 0) {                                                                              \
            answer = renderer->FatalError("Could not read file: %s ", path);                        \
            if (answer == 2) {                                                                      \
                CLEANUP                                                                             \
                CloseHandle(file);                                                                  \
                return 0;                                                                           \
            }                                                                                       \
        }                                                                                           \
    }

LPDIRECTDRAWSURFACE Bitmap::Load(const char *path)
{
    // These temporaries are used in successive stages of the load.
    struct {
        union { int tmp[6]; int paletteBytes; int pixelBytes; long pitch; };
        BITMAPINFOHEADER bi;
        unsigned char fh[14];
        RenderFmt fmt;
        DDSURFACEDESC ld;
        unsigned char planes[768];
    } v;
    HANDLE file = 0;
    int answer = 1;
    while (file == 0 || answer == 4) {
        file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
        if (file == 0) {
            answer = renderer->FatalError("Could not read file: %s ", path);
            if (answer == 2)
                return 0;
        }
    }
    DWORD got;
    int rc;
    READ_RETRY(v.fh, 14, )
    READ_RETRY(&v.bi, 40, )
    v.paletteBytes = (1 << v.bi.biBitCount) * 4;
    BITMAPINFOHEADER *info = (BITMAPINFOHEADER *)malloc(v.paletteBytes + 40);
    if (!info) {
        renderer->FatalError("Can't allocate memory for\n%s", path);
        CloseHandle(file);
        return 0;
    }
    info->biSize = v.bi.biSize;
    info->biWidth = v.bi.biWidth;
    info->biHeight = v.bi.biHeight;
    info->biPlanes = v.bi.biPlanes;
    info->biBitCount = v.bi.biBitCount;
    info->biCompression = v.bi.biCompression;
    info->biSizeImage = v.bi.biSizeImage;
    info->biXPelsPerMeter = v.bi.biXPelsPerMeter;
    info->biYPelsPerMeter = v.bi.biYPelsPerMeter;
    info->biClrUsed = v.bi.biClrUsed;
    info->biClrImportant = v.bi.biClrImportant;
    READ_RETRY((unsigned char *)info + 40, v.paletteBytes, free(info);)
    v.pixelBytes = *(int *)(v.fh + 2) - *(int *)(v.fh + 10);
    unsigned char *pixels = (unsigned char *)malloc(v.pixelBytes);
    if (!pixels) {
        renderer->FatalError("Can't allocate memory for\n%s", path);
        free(info);
        CloseHandle(file);
        return 0;
    }
    READ_RETRY(pixels, v.pixelBytes, free(pixels); free(info);)
    width = (short)info->biWidth;
    height = (short)info->biHeight;
    surface = CreateSurface();
    if (!surface) {
        free(pixels);
        free(info);
        CloseHandle(file);
        return 0;
    }
    surface->Restore();
    v.ld.dwSize = sizeof(v.ld);
    if (surface->Lock(0, &v.ld, DDLOCK_WAIT, 0) != 0) {
        renderer->FatalError("Can't lock surface for\n%s", path);
        surface->Release();
        surface = 0;
        free(pixels);
        free(info);
        CloseHandle(file);
        return 0;
    }
    v.fmt = *(RenderFmt *)renderer->GetPixelFormat(v.tmp);
    unsigned char *entry = (unsigned char *)info + 40;
    for (int i = 0; i < 256; i++, entry += 4) {
        v.planes[i] = entry[2];
        v.planes[256 + i] = entry[1];
        v.planes[512 + i] = entry[0];
    }
    v.pitch = v.ld.lPitch;
    int stride = (width + 3) & ~3;
    int offset = 0;
    for (int y = 0; y < height;) {
        unsigned char *src = pixels + (height - 1 - y) * stride;
        y++;
        ImageConvertRow((unsigned char *)v.ld.lpSurface + offset, &v.fmt, src, v.planes, width);
        offset += v.pitch;
    }
    surface->Unlock(v.ld.lpSurface);
    free(pixels);
    free(info);
    CloseHandle(file);
    SetColorKey(surface, 0xffffffff);
    return surface;
}

