// Renderer::EnterDisplayMode (0x476250).
#include "Renderer2/Renderer2.h"

int Renderer::EnterDisplayMode(int w, int h)
{
    HRESULT hr;
    if (displayState == 1 && fullscreen)
        return 0;
    if (displayState == 2) {
        if (!fullscreen)
            return 0;
    }
    if (fullscreen) {
        if (!SetCooperativeNormalExclusive(ddraw))
            return 0;
        hr = ddraw->SetDisplayMode(640, 480, fmt.bpp);
        if (FAILED(hr)) {
            Error("SetDisplayMode failed.\n%s", HResultToString(hr));
            return 0;
        }
        PositionFullscreen();
        displayState = 1;
    } else {
        int dw, dh, dbpp;
        QueryDesktopMode(&dw, &dh, &dbpp);
        if (savedWidth != dw || savedHeight != dh || fmt.bpp != dbpp) {
            if (!SetCooperativeNormalExclusive(ddraw))
                return 0;
            if (!SetDisplayModeChecked(savedWidth, savedHeight, fmt.bpp))
                return 0;
        }
        if (!SetCooperativeNormal((int)ddraw))
            return 0;
        PositionWindowed(w, h);
        displayState = 2;
    }
    return hr;
}
