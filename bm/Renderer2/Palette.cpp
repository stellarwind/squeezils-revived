// Renderer palette: SetPaletteEntries (0x474c00), UploadFrontPalette (0x474e10).
#include <string.h>
#include "Renderer2/Renderer2.h"

int Renderer::SetPaletteEntries(int first, int count, const unsigned char *rgba)
{
    for (int i = 0; i < count; i++) {
        palette[first + i][0] = rgba[i * 4 + 0];
        palette[first + i][1] = rgba[i * 4 + 1];
        palette[first + i][2] = rgba[i * 4 + 2];
    }
    if (device != 0 && initializing == 0) {
        int h = doubleSize ? height * 2 : height;
        int w = doubleSize ? width * 2 : width;
        return Initialize(windowObj, hwnd, originX, originY, w, h, requestedWidth, requestedHeight, fmt.bpp, fullscreen, doubleSize, 0);
    }
    return 1;
}

int Renderer::UploadFrontPalette()
{
    if (frontPalette == 0)
        return 1;
    PALETTEENTRY out[256];
    int i;
    if (fullscreen != 0) {
        out[0].peRed = 0;
        out[0].peGreen = 0;
        out[0].peBlue = 0;
        out[0].peFlags = 0x44;
        for (i = 1; i < 255; i++) {
            out[i].peFlags = 0x44;
            out[i].peRed = palette[i][0];
            out[i].peGreen = palette[i][1];
            out[i].peBlue = palette[i][2];
        }
        out[255].peFlags = 0x44;
        out[255].peRed = 0xff;
        out[255].peGreen = 0xff;
        out[255].peBlue = 0xff;
    } else {
        for (i = 0; i < 236; i++) {
            out[i].peFlags = 0x44;
            out[i].peRed = palette[10 + i][0];
            out[i].peGreen = palette[10 + i][1];
            out[i].peBlue = palette[10 + i][2];
        }
        for (; i < 255; i++) {
            out[i].peFlags = 0;
            out[i].peRed = 0;
            out[i].peGreen = 0;
            out[i].peBlue = 0;
        }
    }
    HRESULT hr = frontPalette->SetEntries(0, 0, 256, out);
    if (FAILED(hr)) {
        Error("SetEntries failed (front).\n%s", HResultToString(hr));
        return 0;
    }
    return 1;
}
