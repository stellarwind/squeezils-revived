// Renderer.cpp: hand-translated from the Ghidra decompile of SQZ.EXE 0x474230-0x476a00.
// Goal: readable code that behaves like the original; not byte-identical.
// Conventions: // @ADDR = original function address.
#include "Renderer.h"
#include "ListenerTable.h"
#include <stdio.h>
#include <stdarg.h>
#include <mmsystem.h>
#include <math.h>

// Still taken from the original binary:
extern const char *HResultToString(HRESULT hr);                       // @00478430
extern int  PrepareForDialog();                                       // @004753c0 (returns 0)
static void *const kWindowCallback = (void *)0x476630;                // callback registered with the window hook

// ---- desktop helpers ----------------------------------------------------------------------

// @00474360
void QueryDesktopMode(int *width, int *height, int *bpp)
{
    HDC dc = GetDC(NULL);
    *width  = GetDeviceCaps(dc, HORZRES);
    *height = GetDeviceCaps(dc, VERTRES);
    *bpp    = GetDeviceCaps(dc, BITSPIXEL);
    ReleaseDC(NULL, dc);
}

// @004743b0  The game only runs on 8..16 bpp desktops. (The proxy ddraw.dll works around this.)
BOOL CheckDesktopDepth(int *width, int *height)
{
    int bpp;
    QueryDesktopMode(width, height, &bpp);
    if (bpp >= 8 && bpp <= 16)
        return TRUE;
    MessageBoxA(NULL, "This application requires that your Display Properties\nare set to either 256 Colors or High Color (16-bit).\nPlease update your Display Properties with the Display Control Panel.", "Error", MB_OK);
    return FALSE;
}

// @00474230  Reads back the pixel value the surface uses for `color` (SetPixel, Lock, mask).
// Returns 0xffffffff when color == 0xffffffff (no key).
// NOTE: the mask (1 << bitsPerPixel) - 1 is wrong at 32 bpp (shift count wraps) (the proxy ddraw.dll reports 31 bits for that case).
DWORD GetColorKeyValue(LPDIRECTDRAWSURFACE surf, COLORREF color)
{
    DWORD result = 0xffffffff;
    HDC dc;
    COLORREF saved = 0;

    if (color != 0xffffffff) {
        if (surf->GetDC(&dc) == DD_OK) {
            saved = GetPixel(dc, 0, 0);
            SetPixel(dc, 0, 0, color);
            surf->ReleaseDC(dc);
        }
    }

    DDSURFACEDESC desc;
    memset(&desc, 0, sizeof desc);
    desc.dwSize = sizeof desc;
    HRESULT hr;
    do {
        hr = surf->Lock(NULL, &desc, 0, NULL);
    } while (hr == DDERR_WASSTILLDRAWING);
    if (hr == DD_OK) {
        result = ((1u << (desc.ddpfPixelFormat.dwRGBBitCount & 0x1f)) - 1u) & *(DWORD *)desc.lpSurface;
        surf->Unlock(NULL);
    }

    if (color != 0xffffffff) {
        // restore the pixel we overwrote
        if (surf->GetDC(&dc) == DD_OK) {
            SetPixel(dc, 0, 0, saved);
            surf->ReleaseDC(dc);
        }
    }
    return result;
}

// @00474310
void SetSourceColorKey(LPDIRECTDRAWSURFACE surf, COLORREF color)
{
    DDCOLORKEY key;
    key.dwColorSpaceLowValue  = GetColorKeyValue(surf, color);
    key.dwColorSpaceHighValue = key.dwColorSpaceLowValue;
    surf->SetColorKey(DDCKEY_SRCBLT, &key);
}

// ---- construction -------------------------------------------------------------------------

// @00474400
Renderer::Renderer()
{
    windowObj = NULL; initializing = 0; hwnd = NULL; fullscreen = 0;
    doubleSize = 0; ddraw = NULL; d3d = NULL; d3drm = NULL; clipper = NULL; depthBitMask = 0;
    SetRect(&screenRect, 0, 0, 0, 0);
    originX = originY = width = height = 0;

    // default palette: entries 10..245 are a grey ramp, others untouched here (0 from the memset the game does)
    memset(palette, 0, sizeof palette);
    for (int i = 0; i < 236; i++) {
        unsigned char v = (unsigned char)((i * 0xff) / 0xeb);
        palette[10 + i][0] = palette[10 + i][1] = palette[10 + i][2] = v;
    }
    deviceFound = 0;
    chosenDevice = -1;
    hardware = 0;
    monoColorModel = 1;
    sceneFrame = cameraFrame = NULL;
    primary = backBuffer = depth = NULL;
    frontPalette = backPalette = NULL;
    device = NULL;
    textureColors = textureShades = shades = 8;
    viewport = NULL;
    nearPlane = 1.0f;
    farPlane = 1000.0f;
    fovDegrees = 90.0f;
    ownsDDObjects = 0;
    field_078 = 0;
    savedWidth = savedHeight = -1;
    requestedWidth = requestedHeight = 0;
    displayState = DISPLAY_NONE;
}

// ---- messages -----------------------------------------------------------------------------

// @004754c0
int Renderer::Error(const char *fmt, ...)
{
    char text[256];
    va_list ap; va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    PrepareForDialog();
    return MessageBoxA(hwnd, text, "Note", MB_ICONINFORMATION);
}

// @00475510
int Renderer::FatalError(const char *fmt, ...)
{
    char text[256];
    va_list ap; va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    PrepareForDialog();
    return MessageBoxA(hwnd, text, "Application Error", MB_RETRYCANCEL | MB_ICONERROR);
}

// ---- initialisation -----------------------------------------------------------------------

// @00474540
// Sets up the window hook, DirectDraw, Direct3D, D3DRM, the clipper, picks a D3D device, builds the
// surfaces/palette/frames/viewport. On any failure Shutdown(FALSE) is called and FALSE returned.
BOOL Renderer::Initialize(void *wndObj, HWND wnd, int x, int y, int w, int h,
                          int reqW, int reqH, int /*unusedBpp*/, BOOL full, BOOL dbl,
                          const unsigned char *rgbaPalette)
{
    // If the display mode (fullscreen flag) is unchanged only keep the DD objects, otherwise tear down.
    Shutdown((fullscreen == full) ? FALSE : (displayState != 0));
    initializing = 1;

    // register our window callback once (f20 Contains, ef0 Add)
    ListenerTable *listeners = (ListenerTable *)wndObj;
    if (!listeners->Contains(kWindowCallback, this) && !listeners->Add(kWindowCallback, this))
        return FALSE;
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
    if (ddraw == NULL && FAILED(hr = DirectDrawCreate(NULL, &ddraw, NULL))) {
        Error("DirectDrawCreate failed.\n%s", HResultToString(hr));
        Shutdown(FALSE);
        return FALSE;
    }
    if (d3d == NULL && FAILED(hr = ddraw->QueryInterface(IID_IDirect3D, (void **)&d3d))) {
        Error("QueryInterface failed. (Direct3D).\n%s", HResultToString(hr));
        Shutdown(FALSE);
        return FALSE;
    }
    if (d3drm == NULL && FAILED(hr = Direct3DRMCreate(&d3drm))) {
        Error("Direct3DRMCreate failed.\n%s", HResultToString(hr));
        Shutdown(FALSE);
        return FALSE;
    }

    QueryDisplayMode();
    EnterDisplayMode(reqW, reqH);
    requestedWidth = reqW;
    requestedHeight = reqH;

    if (!fullscreen) {
        if (FAILED(hr = DirectDrawCreateClipper(0, &clipper, NULL))) {
            Error("CreateClipper failed.\n%s", HResultToString(hr));
            Shutdown(FALSE);
            return FALSE;
        }
        if (FAILED(hr = clipper->SetHWnd(0, hwnd))) {
            Error("SetHWnd failed (clipper).\n%s", HResultToString(hr));
            Shutdown(FALSE);
            return FALSE;
        }
    }

    // DDBD_* mask for the render target depth: DDBD_1 .. DDBD_32 by display bpp
    switch (fmt.bpp) {
    case 1:    depthBitMask = DDBD_1;  break;   // 0x4000
    case 2:    depthBitMask = DDBD_2;  break;   // 0x2000
    case 4:    depthBitMask = DDBD_4;  break;   // 0x1000
    case 8:    depthBitMask = DDBD_8;  break;   // 0x0800
    case 16:   depthBitMask = DDBD_16; break;   // 0x0400
    case 24:   depthBitMask = DDBD_24; break;   // 0x0200
    case 32:   depthBitMask = DDBD_32; break;   // 0x0100
    default:   Shutdown(FALSE); return FALSE;
    }

    if (!EnumerateDevices()) {
        Shutdown(FALSE);
        return FALSE;
    }
    if (hardware)
        doubleSize = 0;           // hardware renders full size, no 2x stretch

    originX = x;
    originY = y;
    width  = doubleSize ? w / 2 : w;
    height = doubleSize ? h / 2 : h;
    UpdateScreenRect();

    if (rgbaPalette)
        SetPaletteEntries(0, 0x100, rgbaPalette);

    // Seed the first and last 10 palette entries with the system palette (windowed palette mode).
    HDC dc = GetDC(hwnd);
    GetSystemPaletteEntries(dc, 0,    10, (LPPALETTEENTRY)&palette[0][0]);
    GetSystemPaletteEntries(dc, 0xf6, 10, (LPPALETTEENTRY)&palette[246][0]);
    ReleaseDC(hwnd, dc);

    if (!CreateFrames() || !CreateSurfaces()) {
        Shutdown(FALSE);
        return FALSE;
    }
    ApplyQualitySettings();
    if (!CreateViewport()) {
        Shutdown(FALSE);
        return FALSE;
    }
    UpdateViewportPlanes();
    initializing = 0;
    return TRUE;
}

// @00474930  Releases everything. `forceRelease` also releases the primary surface and front palette.
BOOL Renderer::Shutdown(BOOL forceRelease)
{
    if (viewport)  { viewport->Release();  viewport = NULL; }
    if (frontPalette && (ownsDDObjects || forceRelease)) { frontPalette->Release(); frontPalette = NULL; }
    if (backPalette)  { backPalette->Release();  backPalette = NULL; }
    if (primary && (ownsDDObjects || forceRelease)) { primary->Release(); primary = NULL; }
    if (backBuffer)   { backBuffer->Release();   backBuffer = NULL; }
    if (depth)        { depth->Release();        depth = NULL; }
    if (device)       { device->Release();       device = NULL; }
    if (ownsDDObjects) {
        if (cameraFrame) { cameraFrame->Release(); cameraFrame = NULL; }
        if (sceneFrame)  { sceneFrame->Release();  sceneFrame = NULL; }
    }
    hardware = 0;
    deviceFound = 0;
    monoColorModel = 1;
    chosenDevice = -1;

    if (ownsDDObjects && fullscreen) {
        ddraw->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
        ddraw->RestoreDisplayMode();
        fullscreen = 0;
    }
    if (clipper) { clipper->Release(); clipper = NULL; }
    if (ownsDDObjects) {
        if (d3drm) { d3drm->Release(); d3drm = NULL; }
        if (d3d)   { d3d->Release();   d3d = NULL; }
        if (ddraw) { ddraw->Release(); ddraw = NULL; }
    }
    originX = originY = 0;
    width = height = 0;
    depthBitMask = 0;
    SetRect(&screenRect, 0, 0, 0, 0);
    doubleSize = 0;
    hwnd = NULL;
    if (ownsDDObjects && windowObj) {
        ((ListenerTable *)windowObj)->Remove(kWindowCallback, this);
        windowObj = NULL;
    }
    return TRUE;
}

// ---- display mode and window --------------------------------------------------------------

// @00475560  Reads the current display mode into `fmt`. Palettized 4/8 bpp or RGB only.
BOOL Renderer::QueryDisplayMode()
{
    DDSURFACEDESC desc;
    memset(&desc, 0, sizeof desc);
    desc.dwSize = sizeof desc;
    HRESULT hr = ddraw->GetDisplayMode(&desc);
    if (FAILED(hr)) {
        Error("GetDisplayMode failed.\n%s", HResultToString(hr));
        Shutdown(FALSE);
        return FALSE;
    }
    DWORD f = desc.ddpfPixelFormat.dwFlags;
    if (f & DDPF_PALETTEINDEXED1) {
        Error("Can't work with 1-bit palettized surface.");
        Shutdown(FALSE);
        return FALSE;
    }
    if (f & DDPF_PALETTEINDEXED2) {
        Error("Can't work with 2-bit palettized surface.");
        Shutdown(FALSE);
        return FALSE;
    }
    if (f & DDPF_PALETTEINDEXED4) { fmt.bpp = 4; fmt.isPalettized = 1; return TRUE; }
    if (f & DDPF_PALETTEINDEXED8) { fmt.bpp = 8; fmt.isPalettized = 1; return TRUE; }
    if (f & DDPF_RGB) {
        fmt.bpp       = desc.ddpfPixelFormat.dwRGBBitCount;
        fmt.redMask   = desc.ddpfPixelFormat.dwRBitMask;
        fmt.greenMask = desc.ddpfPixelFormat.dwGBitMask;
        fmt.blueMask  = desc.ddpfPixelFormat.dwBBitMask;
        fmt.isPalettized = 0;
        fmt.alphaMask = desc.ddpfPixelFormat.dwRGBAlphaBitMask;
        return TRUE;
    }
    Error("Surface is not palettized nor RGB.");
    Shutdown(FALSE);
    return FALSE;
}

// @00476250  Switches to the display mode/window layout for the current fullscreen setting.
// Windowed: restore the desktop mode if it changed, normal cooperative level, centre the window.
// Fullscreen: exclusive cooperative level, 640x480.
int Renderer::EnterDisplayMode(int w, int h)
{
    if (displayState == DISPLAY_FULLSCREEN && fullscreen)
        return 0;
    if (displayState == DISPLAY_WINDOWED) {
        if (!fullscreen)
            return 0;
    } else if (!fullscreen) {
        int dw, dh, dbpp;
        QueryDesktopMode(&dw, &dh, &dbpp);
        if (savedWidth != dw || savedHeight != dh || fmt.bpp != dbpp) {
            if (!SetCooperativeNormalExclusive()) return 0;
            if (!SetDisplayModeChecked(savedWidth, savedHeight, fmt.bpp)) return 0;
        }
        if (!SetCooperativeNormal()) return 0;
        BOOL r = PositionWindowed(w, h);          // original returns the uninitialised local here (decompile artefact)
        displayState = DISPLAY_WINDOWED;
        return r;
    }
    if (!SetCooperativeNormalExclusive()) return 0;
    HRESULT hr = ddraw->SetDisplayMode(640, 480, fmt.bpp);
    if (FAILED(hr)) {
        Error("SetDisplayMode failed.\n%s", HResultToString(hr));
        return 0;
    }
    PositionFullscreen();
    displayState = DISPLAY_FULLSCREEN;
    return hr;
}

// @004763c0  DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE... flags 0x11 = DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN
BOOL Renderer::SetCooperativeNormalExclusive()
{
    HRESULT hr = ddraw->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if (FAILED(hr)) {
        Error("SetCooperativeLevel failed.\n%s", HResultToString(hr));
        Shutdown(FALSE);
        return FALSE;
    }
    return TRUE;
}

// @00476410  flags 8 = DDSCL_NORMAL
BOOL Renderer::SetCooperativeNormal()
{
    HRESULT hr = ddraw->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
    if (FAILED(hr)) {
        Error("SetCooperativeLevel failed.\n%s", HResultToString(hr));
        Shutdown(FALSE);
        return FALSE;
    }
    return TRUE;
}

// @00476460
BOOL Renderer::SetDisplayModeChecked(int w, int h, int bpp)
{
    HRESULT hr = ddraw->SetDisplayMode(w, h, bpp);
    if (FAILED(hr)) {
        Error("could not set display mode.\n%s", HResultToString(hr));   // string at 004c1960
        Shutdown(FALSE);
        return FALSE;
    }
    return TRUE;
}

// @004764c0  Windowed: caption window, centred on the desktop, client area w x h.
BOOL Renderer::PositionWindowed(int w, int h)
{
    SetLastError(0);
    LONG style = GetWindowLongA(hwnd, GWL_STYLE);
    int dw, dh, dbpp;
    QueryDesktopMode(&dw, &dh, &dbpp);
    style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;     // 0xca0000
    SetWindowLongA(hwnd, GWL_STYLE, style);

    RECT r;
    r.left = dw / 2 - w / 2;
    r.top  = dh / 2 - h / 2;
    if (r.top  < 1) r.top  += 20;
    if (r.left < 1) r.left += 2;
    r.right  = r.left + w;
    r.bottom = r.top + h;
    AdjustWindowRectEx(&r, style, FALSE, WS_EX_DLGMODALFRAME);
    SetWindowPos(hwnd, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_FRAMECHANGED | SWP_NOZORDER);
    return TRUE;
}

// @004765d0  Fullscreen: borderless 640x480 at 0,0.
BOOL Renderer::PositionFullscreen()
{
    SetLastError(0);
    LONG style = GetWindowLongA(hwnd, GWL_STYLE);
    SetWindowLongA(hwnd, GWL_STYLE, style & 0xff34ffff);   // clears CAPTION, SYSMENU, MIN/MAXBOX
    SetWindowPos(hwnd, NULL, 0, 0, 640, 480, SWP_FRAMECHANGED | SWP_NOZORDER);
    return TRUE;
}

// @00475870  Screen-space rectangle of the render area (client origin + offset, or 0,0 in fullscreen).
void Renderer::UpdateScreenRect()
{
    int w = doubleSize ? width * 2 : width;
    int h = doubleSize ? height * 2 : height;
    if (!fullscreen) {
        POINT p = { 0, 0 };
        ClientToScreen(hwnd, &p);
        p.x += originX;
        p.y += originY;
        SetRect(&screenRect, p.x, p.y, p.x - 1 + w, p.y - 1 + h);
        return;
    }
    SetRect(&screenRect, 0, 0, w - 1, h - 1);
}

// @00474f70  Screen rect converted to client coordinates: out = { left, top, right, bottom }.
void Renderer::GetClientRectInWindow(LONG out[4])
{
    POINT tl = { screenRect.left,  screenRect.top };
    POINT br = { screenRect.right, screenRect.bottom };
    ScreenToClient(hwnd, &tl);
    ScreenToClient(hwnd, &br);
    out[0] = tl.x; out[1] = tl.y; out[2] = br.x; out[3] = br.y;
}

// @00474fe0
void Renderer::GetScreenRect(LONG out[4])
{
    out[0] = screenRect.left; out[1] = screenRect.top; out[2] = screenRect.right; out[3] = screenRect.bottom;
}

// @00475000 and @00475020 (identical): copy of bpp, palettized flag, RGB masks and alpha mask.
int *Renderer::GetPixelFormat(int out[6])
{
    memcpy(out, &fmt, sizeof fmt);
    return out;       // the original returns its destination (callers use EAX)
}

// ---- device selection ---------------------------------------------------------------------

// @00475ad0  (named EnumDeviceCallback in Ghidra)
BOOL Renderer::EnumerateDevices()
{
    struct Thunk {
        static HRESULT WINAPI Callback(GUID *guid, LPSTR /*desc*/, LPSTR name, LPD3DDEVICEDESC hw, LPD3DDEVICEDESC sw, LPVOID ctx)
        {
            return ((Renderer *)ctx)->OnEnumDevice(guid, name, hw, sw) ? D3DENUMRET_OK : D3DENUMRET_CANCEL;
        }
    };
    chosenDevice = -1;
    HRESULT hr = d3d->EnumDevices(Thunk::Callback, this);
    if (FAILED(hr)) {
        Error("EnumDevices failed.\n%s", HResultToString(hr));
        return FALSE;
    }
    if (deviceFound == 0) {
        Error("Can't find a compatible Direct3D driver.");
        return FALSE;
    }
    return TRUE;
}

// @00475940  Called once per Direct3D device. Remembers every acceptable device (up to 16) and picks one:
//   - the first acceptable device wins;
//   - hardware beats software; among equals RGB colour model beats mono, except on palettized displays
//     where a mono-capable device is preferred.
// Returns TRUE to continue enumerating (FALSE once the 16-slot table is full).
BOOL Renderer::OnEnumDevice(GUID *guid, const char *name, D3DDEVICEDESC *hw, D3DDEVICEDESC *sw)
{
    D3DDEVICEDESC *desc = (hw->dcmColorModel == 0) ? sw : hw;
    bool isHardware = (desc == hw);

    // skip devices that cannot render to our display depth
    if ((depthBitMask & desc->dwDeviceRenderBitDepth) == 0)
        return TRUE;

    deviceGuids[deviceFound] = *guid;
    strcpy(deviceNames[deviceFound], name);

    bool pick = false;
    int  mono = 0;                      // value for monoColorModel when picked
    if (chosenDevice == -1) {
        pick = true; mono = desc->dcmColorModel & D3DCOLOR_MONO;
    } else if (isHardware && hardware == 0) {
        pick = true; mono = desc->dcmColorModel & D3DCOLOR_MONO;     // upgrade software -> hardware
    } else if (!isHardware && hardware != 0) {
        // keep the hardware device
    } else if (!isHardware && fmt.isPalettized) {
        if (desc->dcmColorModel & D3DCOLOR_MONO) { pick = true; mono = 1; }
    } else if (!fmt.isPalettized) {
        if (desc->dcmColorModel & D3DCOLOR_RGB)  { pick = true; mono = 0; }
    }
    if (pick) {
        chosenDevice = deviceFound;
        hardware = isHardware ? 1 : 0;
        monoColorModel = mono;
    }
    deviceFound++;
    return deviceFound != MAX_DEVICES;
}

// ---- surfaces, palettes, D3DRM objects ----------------------------------------------------

// Fills `out` with the 256 PALETTEENTRYs for the front (screen) palette from `palette`.
// Windowed: entries 10..245 of the working palette go to slots 0..235 (the system keeps the rest).
// Fullscreen: slot 0 black, slots 1..254 straight copies, slot 255 white. peFlags are 0x44 where used.
void Renderer::BuildFrontPaletteEntries(PALETTEENTRY out[256])
{
    memset(out, 0, 256 * sizeof(PALETTEENTRY));
    if (!fullscreen) {
        for (int i = 0; i < 236; i++) {
            out[i].peRed   = palette[10 + i][0];
            out[i].peGreen = palette[10 + i][1];
            out[i].peBlue  = palette[10 + i][2];
            out[i].peFlags = 0x44;
        }
    } else {
        out[0].peFlags = 0x44;
        for (int i = 1; i < 255; i++) {
            out[i].peRed   = palette[i][0];
            out[i].peGreen = palette[i][1];
            out[i].peBlue  = palette[i][2];
            out[i].peFlags = 0x44;
        }
        out[255].peRed = out[255].peGreen = out[255].peBlue = 0xff;
        out[255].peFlags = 0x44;
    }
}

// @00474c00  Copies `count` RGBA entries into the working palette from index `first` and re-initialises
// the renderer if it is already running (original calls Initialize again with the stored settings).
void Renderer::CopyPaletteEntries(int first, int count, const unsigned char *rgba)
{
    for (int i = 0; i < count; i++) {
        palette[first + i][0] = rgba[i * 4 + 0];
        palette[first + i][1] = rgba[i * 4 + 1];
        palette[first + i][2] = rgba[i * 4 + 2];
    }
}

BOOL Renderer::SetPaletteEntries(int first, int count, const unsigned char *rgba)
{
    CopyPaletteEntries(first, count, rgba);
    if (device != NULL && initializing == 0) {
        int w = doubleSize ? width * 2 : width;
        int h = doubleSize ? height * 2 : height;
        return Initialize(windowObj, hwnd, originX, originY, w, h, requestedWidth, requestedHeight,
                          fmt.bpp, fullscreen, doubleSize, NULL);
    }
    return TRUE;
}

// @00474e10
BOOL Renderer::UploadFrontPalette()
{
    if (frontPalette == NULL)
        return TRUE;
    PALETTEENTRY entries[256];
    BuildFrontPaletteEntries(entries);
    HRESULT hr = frontPalette->SetEntries(0, 0, 256, entries);
    if (SUCCEEDED(hr))
        return TRUE;
    Error("SetEntries failed (front).\n%s", HResultToString(hr));
    return FALSE;
}

// @004750a0  Palette fade: ramps the front palette between black and the working palette over a time given
// Fades the 8-bit front palette to black over `seconds`, blocking (busy loop on timeGetTime).
// Fullscreen uses palette entries 1..254 (0 and 255 are fixed); windowed skips the 10 system
// colours at each end and uses entries 10..245 for slots 0..235. Only RGB is scaled, flags stay 0x44.
// In windowed mode the original leaves the RGB of slot 254 and all of slot 255 uninitialised (stack garbage); we zero them.
void Renderer::FadePalette(float seconds)
{
    if (!monoColorModel)
        return;

    DWORD start = timeGetTime();
    DWORD duration = (DWORD)(int)((double)seconds * 1000.0);

    PALETTEENTRY entries[256];
    int first, last, srcFirst;   // slots [first, last) are scaled
    if (fullscreen) {
        first = 1; last = 255; srcFirst = 1;
        for (int i = 0; i < 256; i++) { entries[i].peRed = entries[i].peGreen = entries[i].peBlue = 0; entries[i].peFlags = 0x44; }
        entries[255].peRed = entries[255].peGreen = entries[255].peBlue = 255;
    } else {
        first = 0; last = 236; srcFirst = 10;
        memset(entries, 0, sizeof entries);
        for (int i = 0; i < 236; i++) entries[i].peFlags = 0x44;
    }

    if (duration != 0) {
        float startF = (float)start, durationF = (float)duration;
        DWORD now = start;
        do {
            float scale = (float)(1.0f - ((double)now - startF) / durationF);
            for (int i = first; i < last; i++) {
                const unsigned char *src = palette[srcFirst + (i - first)];
                entries[i].peRed   = (BYTE)(int)((double)src[0] * scale);
                entries[i].peGreen = (BYTE)(int)((double)src[1] * scale);
                entries[i].peBlue  = (BYTE)(int)((double)src[2] * scale);
            }
            now = timeGetTime();
            frontPalette->SetEntries(0, 0, 256, entries);
        } while (now - start < duration);
    }

    for (int i = first; i < last; i++)
        entries[i].peRed = entries[i].peGreen = entries[i].peBlue = 0;
    frontPalette->SetEntries(0, 0, 256, entries);
}

// @00474f40
void Renderer::SetViewParams(float nearP, float farP, float fov)
{
    nearPlane = nearP;
    farPlane = farP;
    fovDegrees = fov;
    UpdateViewportPlanes();
}

// @00475040 / @00475060 / @00475080: change a value then reapply all quality settings
void Renderer::SetShades(int v)        { shades = v;        ApplyQualitySettings(); }
void Renderer::SetTextureColors(int v) { textureColors = v; ApplyQualitySettings(); }
void Renderer::SetTextureShades(int v) { textureShades = v; ApplyQualitySettings(); }

// @004756a0
HRESULT Renderer::ApplyQualitySettings()
{
    HRESULT hr;
    if (FAILED(hr = d3drm->SetDefaultTextureColors(textureColors)))
        Error("SetDefaultTextureColors(%d) failed.\n%s", textureColors, HResultToString(hr));
    if (FAILED(hr = d3drm->SetDefaultTextureShades(textureShades)))
        Error("SetDefaultTextureShades(%d) failed.\n%s", textureShades, HResultToString(hr));
    if (FAILED(hr = device->SetShades(shades)))
        Error("SetShades(%d) failed.\n%s", shades, HResultToString(hr));
    if (FAILED(hr = device->SetDither(FALSE)))
        Error("SetDither failed (false).\n%s", HResultToString(hr));
    if (FAILED(hr = device->SetQuality(D3DRMRENDER_GOURAUD)))          // 0x89 in the binary
        Error("SetQuality failed (gouraud).\n%s", HResultToString(hr));
    if (FAILED(hr = device->SetTextureQuality(hardware != 0 ? D3DRMTEXTURE_LINEAR : D3DRMTEXTURE_NEAREST)))
        Error("SetTextureQuality failed (nearest).\n%s", HResultToString(hr));
    return hr;      // original leaves the last HRESULT in EAX; the setters tail-call this function
}

// @004757d0  Front/back clip planes and a symmetric frustum from the vertical field of view.
void Renderer::UpdateViewportPlanes()
{
    viewport->SetFront(nearPlane);
    viewport->SetBack(farPlane);
    double half = tan((double)fovDegrees * 0.5 * 0.017453292519943);
    double w = half * nearPlane;
    double h = ((double)height / (double)width) * w;
    viewport->SetPlane((float)-w, (float)w, (float)-h, (float)h);
    viewport->SetUniformScaling(FALSE);
}

// @00475b40
BOOL Renderer::CreateFrames()
{
    HRESULT hr;
    if (sceneFrame == NULL && FAILED(hr = d3drm->CreateFrame(NULL, &sceneFrame))) {
        Error("CreateFrame failed (scene).\n%s", HResultToString(hr));
        return FALSE;
    }
    if (cameraFrame == NULL) {
        if (FAILED(hr = d3drm->CreateFrame(sceneFrame, &cameraFrame))) {
            Error("CreateFrame failed (camera).\n%s", HResultToString(hr));
            return FALSE;
        }
        if (FAILED(hr = cameraFrame->SetPosition(sceneFrame, 0, 0, 0))) {
            Error("SetPosition failed (camera).\n%s", HResultToString(hr));
            return FALSE;
        }
    }
    return TRUE;
}

// @004761f0
BOOL Renderer::CreateViewport()
{
    HRESULT hr = d3drm->CreateViewport(device, cameraFrame, 0, 0, width, height, &viewport);
    if (FAILED(hr)) {
        Error("CreateViewport failed.\n%s", HResultToString(hr));
        return FALSE;
    }
    return TRUE;
}

// @00475c10  Primary + back (3D device) + depth surfaces, clipper, palettes, then the D3DRM device.
// The depth buffer's bit depth is the best one the driver offers: dwZBufferBitDepths of the HAL caps (HEL when the device
// is software), preferring 16 > 24 > 32 > 8, and 16 when it reports none (0x475db3).
BOOL Renderer::CreateSurfaces()
{
    HRESULT hr;
    DDSURFACEDESC sd;

    // primary surface
    memset(&sd, 0, sizeof sd);
    sd.dwSize = sizeof sd;
    sd.dwFlags = DDSD_CAPS;
    sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    if (primary == NULL && FAILED(hr = ddraw->CreateSurface(&sd, &primary, NULL))) {
        Error("CreateSurface failed (primary).\n%s", HResultToString(hr));
        return FALSE;
    }

    // back buffer, usable as a 3D render target
    sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    sd.dwWidth = width;
    sd.dwHeight = height;
    sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | (hardware ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY);
    if (FAILED(hr = ddraw->CreateSurface(&sd, &backBuffer, NULL))) {
        Error("CreateSurface failed (back).\n%s", HResultToString(hr));
        return FALSE;
    }

    // depth buffer
    DDCAPS halCaps, helCaps;
    memset(&halCaps, 0, sizeof halCaps); halCaps.dwSize = sizeof halCaps;
    memset(&helCaps, 0, sizeof helCaps); helCaps.dwSize = sizeof helCaps;
    if (FAILED(hr = ddraw->GetCaps(&halCaps, &helCaps))) {
        Error("GetCaps failed.\n%s", HResultToString(hr));
        return FALSE;
    }
    DWORD zbd = hardware ? halCaps.dwZBufferBitDepths : helCaps.dwZBufferBitDepths;
    DWORD zdepth = 0;
    if (zbd & DDBD_8)  zdepth = 8;
    if (zbd & DDBD_32) zdepth = 32;
    if (zbd & DDBD_24) zdepth = 24;
    if (zbd & DDBD_16) zdepth = 16;
    if (zdepth == 0)
        zdepth = 16;                 // the original's fallback
    memset(&sd, 0, sizeof sd);
    sd.dwSize = sizeof sd;
    sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_ZBUFFERBITDEPTH;   // 0x47
    sd.dwWidth = width;
    sd.dwHeight = height;
    sd.dwZBufferBitDepth = zdepth;
    sd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | (hardware ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY);
    if (FAILED(hr = ddraw->CreateSurface(&sd, &depth, NULL))) {
        Error("CreateSurface failed (depth).\n%s", HResultToString(hr));
        return FALSE;
    }
    if (FAILED(hr = backBuffer->AddAttachedSurface(depth))) {
        Error("AddAttachedSurface failed.\n%s", HResultToString(hr));
        return FALSE;
    }

    if (!fullscreen && FAILED(hr = primary->SetClipper(clipper))) {
        Error("SetClipper failed.\n%s", HResultToString(hr));
        return FALSE;
    }

    if (fmt.bpp == 8) {
        // palettised display: lock/unlock the back buffer once, then build back + front palettes
        DDSURFACEDESC ld;
        memset(&ld, 0, sizeof ld);
        ld.dwSize = sizeof ld;
        if (FAILED(hr = backBuffer->Lock(NULL, &ld, DDLOCK_WAIT, NULL))) {
            Error("Lock failed (palettizing).\n%s", HResultToString(hr));
            return FALSE;
        }
        backBuffer->Unlock(NULL);

        // peFlags per entry: 0-41 and 246-255 = 0x80, 42-245 = 0x40
        for (int i = 0; i < 10; i++)    palette[i][3] = 0x80;
        for (int i = 10; i < 42; i++)   palette[i][3] = 0x80;
        for (int i = 42; i < 246; i++)  palette[i][3] = 0x40;
        for (int i = 246; i < 256; i++) palette[i][3] = 0x80;

        if (FAILED(hr = ddraw->CreatePalette(0xc, (LPPALETTEENTRY)palette, &backPalette, NULL))) {
            Error("CreatePalette failed (back).\n%s", HResultToString(hr));
            return FALSE;
        }
        if (FAILED(hr = backBuffer->SetPalette(backPalette))) {
            Error("SetPalette failed (back).\n%s", HResultToString(hr));
            return FALSE;
        }

        PALETTEENTRY entries[256];
        BuildFrontPaletteEntries(entries);
        if (frontPalette == NULL && FAILED(hr = ddraw->CreatePalette(0xc, entries, &frontPalette, NULL))) {
            Error("CreatePalette failed (front).\n%s", HResultToString(hr));
            return FALSE;
        }
        if (monoColorModel) {
            // windowed palettes use 236 entries (system keeps 20), fullscreen all 256
            if (FAILED(hr = frontPalette->SetEntries(0, 0, fullscreen ? 256 : 236, entries))) {
                Error("SetEntries failed (front).\n%s", HResultToString(hr));
                return FALSE;
            }
            if (FAILED(hr = primary->SetPalette(frontPalette))) {
                Error("SetPalette failed (front).\n%s", HResultToString(hr));
                return FALSE;
            }
        }
    }

    hr = d3drm->CreateDeviceFromSurface(&deviceGuids[chosenDevice], ddraw, backBuffer, &device);
    if (FAILED(hr)) {
        Error("CreateDeviceFromSurface failed.\n%s", HResultToString(hr));
        return FALSE;
    }
    return TRUE;
}
