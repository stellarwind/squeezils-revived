// Renderer::Initialize (0x474540).
#include "Renderer2/Renderer2.h"

struct WindowListeners {
    int Contains(void *callback, void *context);     // 0x477f20
    int Add(void *callback, void *context);          // 0x477ef0
};
extern void WindowCallback();                        // 0x476630

int Renderer::Initialize(void *wndObj, HWND wnd, int x, int y, int w, int h, int reqW, int reqH, int bpp, int full, int dbl,
                         const unsigned char *rgba)
{
    if (fullscreen == full)
        Shutdown(0);
    else if (displayState == 0)
        Shutdown(0);
    else
        Shutdown(1);
    initializing = 1;
    WindowListeners *listeners = (WindowListeners *)wndObj;
    if (!listeners->Contains(WindowCallback, this) && !listeners->Add(WindowCallback, this))
        return 0;
    windowObj = wndObj;
    hwnd = wnd;
    if (savedWidth == -1 || savedHeight == -1)
        QueryDesktopMode(&savedWidth, &savedHeight, &savedBpp);
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);
    fullscreen = full;
    doubleSize = dbl;
    HRESULT hr;
    if (ddraw == 0 && FAILED(hr = DirectDrawCreate(0, &ddraw, 0))) {
        Error("DirectDrawCreate failed.\n%s", HResultToString(hr));
        Shutdown(0);
        return 0;
    }
    if (d3d == 0 && FAILED(hr = ddraw->QueryInterface(IID_IDirect3D, (void **)&d3d))) {
        Error("QueryInterface failed. (Direct3D).\n%s", HResultToString(hr));
        Shutdown(0);
        return 0;
    }
    if (d3drm == 0 && FAILED(hr = Direct3DRMCreate(&d3drm))) {
        Error("Direct3DRMCreate failed.\n%s", HResultToString(hr));
        Shutdown(0);
        return 0;
    }
    QueryDisplayMode();
    EnterDisplayMode(reqW, reqH);
    requestedWidth = reqW;
    requestedHeight = reqH;
    if (!fullscreen) {
        if (FAILED(hr = DirectDrawCreateClipper(0, &clipper, 0))) {
            Error("CreateClipper failed.\n%s", HResultToString(hr));
            Shutdown(0);
            return 0;
        }
        if (FAILED(hr = clipper->SetHWnd(0, hwnd))) {
            Error("SetHWnd failed (clipper).\n%s", HResultToString(hr));
            Shutdown(0);
            return 0;
        }
    }
    switch (fmt.bpp) {
    case 1: depthBitMask = DDBD_1; break;
    case 2: depthBitMask = DDBD_2; break;
    case 4: depthBitMask = DDBD_4; break;
    case 8: depthBitMask = DDBD_8; break;
    case 16: depthBitMask = DDBD_16; break;
    case 24: depthBitMask = DDBD_24; break;
    case 32: depthBitMask = DDBD_32; break;
    default:
        Shutdown(0);
        return 0;
    }
    if (!EnumerateDevices()) {
        Shutdown(0);
        return 0;
    }
    if (hardware)
        doubleSize = 0;
    originX = x;
    originY = y;
    width = doubleSize ? w / 2 : w;
    height = doubleSize ? h / 2 : h;
    UpdateScreenRect();
    if (rgba)
        SetPaletteEntries(0, 0x100, rgba);
    HDC dc = GetDC(hwnd);
    GetSystemPaletteEntries(dc, 0, 10, (LPPALETTEENTRY)&palette[0][0]);
    GetSystemPaletteEntries(dc, 0xf6, 10, (LPPALETTEENTRY)&palette[246][0]);
    ReleaseDC(hwnd, dc);
    if (!CreateFrames()) {
        Shutdown(0);
        return 0;
    }
    if (!CreateSurfaces()) {
        Shutdown(0);
        return 0;
    }
    ApplyQualitySettings();
    if (!CreateViewport()) {
        Shutdown(0);
        return 0;
    }
    UpdateViewportPlanes();
    initializing = 0;
    return 1;
}
