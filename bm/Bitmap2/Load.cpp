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
    unsigned char fh[14];
    BITMAPINFOHEADER bi;
    DWORD got;
    int rc;
    READ_RETRY(fh, 14, )
    READ_RETRY(&bi, 40, )
    int paletteBytes = (1 << bi.biBitCount) * 4;
    BITMAPINFOHEADER *info = (BITMAPINFOHEADER *)malloc(paletteBytes + 40);
    if (!info) {
        renderer->FatalError("Can't allocate memory for\n%s", path);
        CloseHandle(file);
        return 0;
    }
    info->biSize = bi.biSize;
    info->biWidth = bi.biWidth;
    info->biHeight = bi.biHeight;
    info->biPlanes = bi.biPlanes;
    info->biBitCount = bi.biBitCount;
    info->biCompression = bi.biCompression;
    info->biSizeImage = bi.biSizeImage;
    info->biXPelsPerMeter = bi.biXPelsPerMeter;
    info->biYPelsPerMeter = bi.biYPelsPerMeter;
    info->biClrUsed = bi.biClrUsed;
    info->biClrImportant = bi.biClrImportant;
    READ_RETRY((unsigned char *)info + 40, paletteBytes, free(info);)
    int pixelBytes = *(int *)(fh + 2) - *(int *)(fh + 10);
    unsigned char *pixels = (unsigned char *)malloc(pixelBytes);
    if (!pixels) {
        renderer->FatalError("Can't allocate memory for\n%s", path);
        free(info);
        CloseHandle(file);
        return 0;
    }
    READ_RETRY(pixels, pixelBytes, free(pixels); free(info);)
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
    DDSURFACEDESC ld;
    ld.dwSize = sizeof(ld);
    if (surface->Lock(0, &ld, DDLOCK_WAIT, 0) != 0) {
        renderer->FatalError("Can't lock surface for\n%s", path);
        surface->Release();
        surface = 0;
        free(pixels);
        free(info);
        CloseHandle(file);
        return 0;
    }
    int tmp[6];
    RenderFmt fmt = *(RenderFmt *)renderer->GetPixelFormat(tmp);
    unsigned char planes[768];
    unsigned char *entry = (unsigned char *)info + 40;
    for (int i = 0; i < 256; i++, entry += 4) {
        planes[i] = entry[2];
        planes[256 + i] = entry[1];
        planes[512 + i] = entry[0];
    }
    int stride = (width + 3) & ~3;
    int offset = 0;
    for (int y = 0; y < height; y++) {
        ImageConvertRow((unsigned char *)ld.lpSurface + offset, &fmt, pixels + (height - 1 - y) * stride, planes, width);
        offset += ld.lPitch;
    }
    surface->Unlock(ld.lpSurface);
    free(pixels);
    free(info);
    CloseHandle(file);
    SetColorKey(surface, 0xffffffff);
    return surface;
}
