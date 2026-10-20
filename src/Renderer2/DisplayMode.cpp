// Renderer::QueryDisplayMode (0x475560): reads the current display mode into `fmt`; palettized 4/8 bpp or RGB only.
#include <string.h>
#include "Renderer2/Renderer2.h"

void Renderer::QueryDisplayMode()
{
    DDSURFACEDESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    HRESULT hr = ddraw->GetDisplayMode(&desc);
    if (FAILED(hr)) {
        Error("GetDisplayMode failed.\n%s", HResultToString(hr));
        Shutdown(0);
        return;
    }
    DWORD f = desc.ddpfPixelFormat.dwFlags;
    if (f & DDPF_PALETTEINDEXED1) {
        Error("Can't work with 1-bit palettized surface.");
        Shutdown(0);
        return;
    }
    if (desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2) {
        Error("Can't work with 2-bit palettized surface.");
        Shutdown(0);
        return;
    }
    if (desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4) {
        fmt.bpp = 4;
        fmt.isPalettized = 1;
        return;
    }
    if (desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
        fmt.bpp = 8;
        fmt.isPalettized = 1;
        return;
    }
    if (desc.ddpfPixelFormat.dwFlags & DDPF_RGB) {
        fmt.bpp = desc.ddpfPixelFormat.dwRGBBitCount;
        fmt.redMask = desc.ddpfPixelFormat.dwRBitMask;
        fmt.greenMask = desc.ddpfPixelFormat.dwGBitMask;
        fmt.blueMask = desc.ddpfPixelFormat.dwBBitMask;
        fmt.isPalettized = 0;
        fmt.alphaMask = desc.ddpfPixelFormat.dwRGBAlphaBitMask;
        return;
    }
    Error("Surface is not palettized nor RGB.");
    Shutdown(0);
}
