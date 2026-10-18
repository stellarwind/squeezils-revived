// Renderer::CreateSurfaces (0x475c10): primary, back (3D target) and depth surfaces, palettes, then the D3DRM device.
#include <string.h>
#include "Renderer2/Renderer2.h"

int Renderer::CreateSurfaces()
{
    HRESULT hr;
    DDSURFACEDESC sd;
    memset(&sd, 0, sizeof(sd));
    sd.dwSize = sizeof(sd);
    sd.dwFlags = DDSD_CAPS;
    sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    if (primary == 0 && FAILED(hr = ddraw->CreateSurface(&sd, &primary, 0))) {
        Error("CreateSurface failed (primary).\n%s", HResultToString(hr));
        return 0;
    }
    sd.dwWidth = width;
    sd.dwHeight = height;
    sd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;
    if (hardware)
        sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
    if (FAILED(hr = ddraw->CreateSurface(&sd, &backBuffer, 0))) {
        Error("CreateSurface failed (back).\n%s", HResultToString(hr));
        return 0;
    }
    DDCAPS halCaps, helCaps;
    memset(&halCaps, 0, sizeof(halCaps));
    memset(&helCaps, 0, sizeof(helCaps));
    halCaps.dwSize = sizeof(halCaps);
    helCaps.dwSize = sizeof(helCaps);
    if (FAILED(hr = ddraw->GetCaps(&halCaps, &helCaps))) {
        Error("GetCaps failed.\n%s", HResultToString(hr));
        return 0;
    }
    DDCAPS *caps = hardware ? &halCaps : &helCaps;
    DWORD zbd = caps->dwZBufferBitDepths;
    int zdepth = 0;
    if (zbd & DDBD_8)
        zdepth = 8;
    if (zbd & DDBD_32)
        zdepth = 32;
    if (zbd & DDBD_24)
        zdepth = 24;
    if (zbd & DDBD_16)
        zdepth = 16;
    if (zdepth == 0) {
#ifdef SQZ_FIX_BUGS
        // SQZ_FIX_BUGS: playable-build fix. The stock EXE gives up here when the driver reports no z-buffer depths
        // (modern drivers report none). Exported/SQZ.EXE patches the 6 bytes at 0x475db7, the start of
        // the FatalError call below (68 80 18 4c 00 56 = push "This display..."; push this), into
        // bd 10 00 00 00 eb 14 = mov ebp,16; jmp 0x475dd2: the depth defaults to 16 bits and setup goes on.
        zdepth = 16;
#else
        FatalError("This display does not support a depth buffer.");
        return 0;
#endif
    }
    memset(&sd, 0, sizeof(sd));
    sd.dwSize = sizeof(sd);
    sd.dwWidth = width;
    sd.dwHeight = height;
    sd.dwZBufferBitDepth = zdepth;
    sd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_ZBUFFERBITDEPTH;
    sd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;
    if (hardware)
        sd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
    if (FAILED(hr = ddraw->CreateSurface(&sd, &depth, 0))) {
        Error("CreateSurface failed (depth).\n%s", HResultToString(hr));
        return 0;
    }
    if (FAILED(hr = backBuffer->AddAttachedSurface(depth))) {
        Error("AddAttachedSurface failed.\n%s", HResultToString(hr));
        return 0;
    }
    if (!fullscreen) {
        if (FAILED(hr = primary->SetClipper(clipper))) {
            Error("SetClipper failed.\n%s", HResultToString(hr));
            return 0;
        }
    }
    if (fmt.bpp == 8) {
        memset(&sd, 0, sizeof(sd));
        sd.dwSize = sizeof(sd);
        if (FAILED(hr = backBuffer->Lock(0, &sd, DDLOCK_WAIT, 0))) {
            Error("Lock failed (palettizing).\n%s", HResultToString(hr));
            return 0;
        }
        backBuffer->Unlock(0);
        int i;
        for (i = 0; i < 10; i++)
            palette[i][3] = 0x80;
        for (i = 10; i < 42; i++)
            palette[i][3] = 0x80;
        for (i = 42; i < 246; i++)
            palette[i][3] = 0x40;
        for (i = 246; i < 256; i++)
            palette[i][3] = 0x80;
        if (FAILED(hr = ddraw->CreatePalette(0xc, (LPPALETTEENTRY)palette, &backPalette, 0))) {
            Error("CreatePalette failed (back).\n%s", HResultToString(hr));
            return 0;
        }
        if (FAILED(hr = backBuffer->SetPalette(backPalette))) {
            Error("SetPalette failed (back).\n%s", HResultToString(hr));
            return 0;
        }
        PALETTEENTRY out[256];
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
        if (frontPalette == 0 && FAILED(hr = ddraw->CreatePalette(0xc, out, &frontPalette, 0))) {
            Error("CreatePalette failed (front).\n%s", HResultToString(hr));
            return 0;
        }
        if (monoColorModel) {
            if (FAILED(hr = frontPalette->SetEntries(0, 0, fullscreen ? 256 : 236, out))) {
                Error("SetEntries failed (front).\n%s", HResultToString(hr));
                return 0;
            }
            if (FAILED(hr = primary->SetPalette(frontPalette))) {
                Error("SetPalette failed (front).\n%s", HResultToString(hr));
                return 0;
            }
        }
    }
    hr = d3drm->CreateDeviceFromSurface(&deviceGuids[chosenDevice], ddraw, backBuffer, &device);
    if (FAILED(hr)) {
        Error("CreateDeviceFromSurface failed.\n%s", HResultToString(hr));
        return 0;
    }
    return 1;
}
