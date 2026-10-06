// Renderer: DirectDraw + Direct3D Retained Mode (v1 interfaces) setup for Squeezils.
// Reconstructed from SQZ.EXE 0x474230-0x476a00. Names are ours; field offsets are the original ones.
// Not a byte-exact reconstruction. See Renderer.cpp for per-function notes ("TODO(opus)" = unresolved).
#pragma once

#define DIRECTDRAW_VERSION 0x0300
#define DIRECT3D_VERSION   0x0500
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>

// Pixel format of the display (DDPIXELFORMAT subset). GetPixelFormat() copies these 6 dwords.
struct DisplayFormat {
    int bpp;          // 0x38
    int isPalettized; // 0x3c: 1 for 4/8 bpp modes
    int redMask;      // 0x40
    int greenMask;    // 0x44
    int blueMask;     // 0x48
    int alphaMask;    // 0x4c
};

class Renderer {
public:
    enum { MAX_DEVICES = 16, DEVICE_NAME_LEN = 64 };
    // displayState values
    enum { DISPLAY_NONE = 0, DISPLAY_FULLSCREEN = 1, DISPLAY_WINDOWED = 2 };

    Renderer();                                   // 0x474400 (resets every field)
    BOOL  Initialize(void *windowObj, HWND hwnd, int x, int y, int width, int height,
                     int requestedWidth, int requestedHeight, int unusedBpp, BOOL fullscreen,
                     BOOL doubleSize, const unsigned char *rgbaPalette);   // 0x474540
    BOOL  Shutdown(BOOL forceRelease);            // 0x474930

    void  CopyPaletteEntries(int first, int count, const unsigned char *rgba);   // first half of 0x474c00 (RGB only)
    BOOL  SetPaletteEntries(int first, int count, const unsigned char *rgba);   // 0x474c00
    BOOL  UploadFrontPalette();                   // 0x474e10
    void  FadePalette();                          // 0x4750a0  TODO(opus): float timing lost in decompile
    void  SetViewParams(float nearPlane, float farPlane, float fovDegrees);     // 0x474f40
    void  GetClientRectInWindow(LONG out[4]);     // 0x474f70
    void  GetScreenRect(LONG out[4]);             // 0x474fe0
    int  *GetPixelFormat(int out[6]);             // 0x475000 / 0x475020 (identical)
    void  SetShades(int shades);                  // 0x475040
    void  SetTextureColors(int colors);           // 0x475060
    void  SetTextureShades(int shades);           // 0x475080

    void  Error(const char *fmt, ...);            // 0x4754c0 (info box)
    void  FatalError(const char *fmt, ...);       // 0x475510 (error box)

    // Original code names this EnumDeviceCallback; it is the D3D device enumeration driver (0x475ad0).
    BOOL  EnumerateDevices();
    // The real callback (0x475940)
    BOOL  OnEnumDevice(GUID *guid, const char *name, D3DDEVICEDESC *hw, D3DDEVICEDESC *sw);

    // public state the rest of the game reads (offsets in comments)
    int   savedWidth;       // 0x00 desktop mode when we started (-1 = not captured yet)
    int   savedHeight;      // 0x04
    int   savedBpp;         // 0x08
    int   fullscreen;       // 0x0c
    int   displayState;     // 0x10
    void *windowObj;        // 0x14 object that owns the window hook (callback 0x476630)
    HWND  hwnd;             // 0x18
    int   initializing;     // 0x1c 1 while Initialize runs
    int   doubleSize;       // 0x20 render at half size and stretch 2x
    LPDIRECTDRAW      ddraw;    // 0x24
    LPDIRECT3D        d3d;      // 0x28
    LPDIRECT3DRM      d3drm;    // 0x2c
    LPDIRECTDRAWCLIPPER clipper; // 0x30
    int   depthBitMask;     // 0x34 DDBD_* mask matching the display bit depth
    DisplayFormat fmt;      // 0x38..0x4f (6 dwords; GetPixelFormat copies exactly these)
    int   requestedWidth;   // 0x50 width/height passed to Initialize before halving
    int   requestedHeight;  // 0x54
    RECT  screenRect;       // 0x58
    int   originX;          // 0x68
    int   originY;          // 0x6c
    int   width;            // 0x70 (halved if doubleSize)
    int   height;           // 0x74
    int   field_078;        // 0x78
    unsigned char palette[256][4]; // 0x7c RGBA entries
    int   deviceFound;      // 0x47c number of devices found by EnumDevices
    GUID  deviceGuids[MAX_DEVICES];                     // 0x480
    char  deviceNames[MAX_DEVICES][DEVICE_NAME_LEN];    // 0x580
    int   chosenDevice;     // 0x980
    int   hardware;         // 0x984 1 if chosen device is HAL
    int   monoColorModel;   // 0x988 1 if device uses D3DCOLOR_MONO
    LPDIRECT3DRMFRAME  sceneFrame;   // 0x98c
    LPDIRECT3DRMFRAME  cameraFrame;  // 0x990
    LPDIRECTDRAWSURFACE primary;     // 0x994
    LPDIRECTDRAWSURFACE backBuffer;  // 0x998
    LPDIRECTDRAWSURFACE depth;       // 0x99c
    LPDIRECTDRAWPALETTE frontPalette; // 0x9a0
    LPDIRECTDRAWPALETTE backPalette;  // 0x9a4
    LPDIRECT3DRMDEVICE  device;       // 0x9a8
    int   textureColors;    // 0x9ac
    int   textureShades;    // 0x9b0
    int   shades;           // 0x9b4
    LPDIRECT3DRMVIEWPORT viewport;    // 0x9b8
    float nearPlane;        // 0x9bc
    float farPlane;         // 0x9c0
    float fovDegrees;       // 0x9c4
    int   ownsDDObjects;    // 0x9c8 (field_9C8) non-zero: Shutdown also releases ddraw/d3d/d3drm and the window hook

private:
    BOOL CreateFrames();                 // 0x475b40
    BOOL CreateSurfaces();               // 0x475c10
    BOOL CreateViewport();               // 0x4761f0
    BOOL QueryDisplayMode();             // 0x475560 (original ignores the result; it calls Shutdown on failure)
    HRESULT ApplyQualitySettings();      // 0x4756a0 (returns the last HRESULT, as the original leaves it in EAX)
    void UpdateViewportPlanes();         // 0x4757d0
    void UpdateScreenRect();             // 0x475870
    int  EnterDisplayMode(int w, int h); // 0x476250
    BOOL SetCooperativeNormalExclusive();// 0x4763c0 (flags 0x11)
    BOOL SetCooperativeNormal();         // 0x476410 (flags 8)
    BOOL SetDisplayModeChecked(int w, int h, int bpp); // 0x476460
    BOOL PositionWindowed(int w, int h); // 0x4764c0
    BOOL PositionFullscreen();           // 0x4765d0
    void BuildFrontPaletteEntries(PALETTEENTRY out[256]);
};

// Free helpers in the original
void  QueryDesktopMode(int *width, int *height, int *bpp);                // 0x474360
BOOL  CheckDesktopDepth(int *width, int *height);                         // 0x4743b0 requires 8..16 bpp
DWORD GetColorKeyValue(LPDIRECTDRAWSURFACE surf, COLORREF color);         // 0x474230
void  SetSourceColorKey(LPDIRECTDRAWSURFACE surf, COLORREF color);        // 0x474310
