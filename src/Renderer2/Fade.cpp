// Renderer::FadePalette (0x4750a0): ramps the front palette to black over `seconds`, blocking on timeGetTime.
#include <windows.h>
#include "Renderer2/Renderer2.h"

void Renderer::FadePalette(float seconds)
{
    PALETTEENTRY out[256];
    int i;
    if (monoColorModel == 0)
        return;
    DWORD start = timeGetTime();
    DWORD now = start;
    DWORD duration = (DWORD)(seconds * 1000.0f);
    if (fullscreen != 0) {
        out[0].peRed = 0;
        out[0].peGreen = 0;
        out[0].peBlue = 0;
        out[0].peFlags = 0x44;
        for (i = 1; i < 255; i++) {
            BYTE r = palette[i][0];
            BYTE g = palette[i][1];
            BYTE b = palette[i][2];
            out[i].peFlags = 0x44;
            out[i].peRed = r;
            out[i].peGreen = g;
            out[i].peBlue = b;
        }
        out[255].peFlags = 0x44;
        out[255].peRed = 0xff;
        out[255].peGreen = 0xff;
        out[255].peBlue = 0xff;
    } else {
        for (i = 0; i < 236; i++) {
            BYTE r = palette[10 + i][0];
            BYTE g = palette[10 + i][1];
            BYTE b = palette[10 + i][2];
            out[i].peFlags = 0x44;
            out[i].peRed = r;
            out[i].peGreen = g;
            out[i].peBlue = b;
        }
        for (; i < 255; i++) {
            out[i].peFlags = 0;
            out[i].peRed = 0;
            out[i].peGreen = 0;
            out[i].peBlue = 0;
        }
    }
    i = 0;
    if (duration != 0) {
        float startF = (float)start;
        float durationF = (float)duration;
        do {
            float scale = (float)(1.0f - (now - startF) / durationF);
            if (fullscreen != 0) {
                for (i = 1; i < 255; i++) {
                    out[i].peRed = (BYTE)(int)((unsigned int)palette[i][0] * scale);
                    out[i].peGreen = (BYTE)(int)((unsigned int)palette[i][1] * scale);
                    out[i].peBlue = (BYTE)(int)((unsigned int)palette[i][2] * scale);
                }
            } else {
                for (i = 0; i < 236; i++) {
                    out[i].peRed = (BYTE)(int)((unsigned int)palette[10 + i][0] * scale);
                    out[i].peGreen = (BYTE)(int)((unsigned int)palette[10 + i][1] * scale);
                    out[i].peBlue = (BYTE)(int)((unsigned int)palette[10 + i][2] * scale);
                }
            }
            now = timeGetTime();
            frontPalette->SetEntries(0, 0, 256, out);
        } while (now - start < duration);
    }
    if (fullscreen != 0) {
        for (i = 1; i < 255; i++) {
            out[i].peRed = 0;
            out[i].peGreen = 0;
            out[i].peBlue = 0;
        }
    } else {
        for (i = 0; i < 236; i++) {
            out[i].peRed = 0;
            out[i].peGreen = 0;
            out[i].peBlue = 0;
        }
    }
    frontPalette->SetEntries(0, 0, 256, out);
}
