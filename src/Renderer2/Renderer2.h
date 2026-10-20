// Renderer (the real class, laid out as in SQZ.EXE) for the byte-match build: DX2 SDK headers, Windows API.
#pragma once
#include "compat41.h"
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>

struct RenderFmt { int bpp, isPalettized, redMask, greenMask, blueMask, alphaMask; };

class Renderer {
public:
    int   savedWidth;       // 0x00
    int   savedHeight;      // 0x04
    int   savedBpp;         // 0x08
    int   fullscreen;       // 0x0c
    int   displayState;     // 0x10
    void *windowObj;        // 0x14
    HWND  hwnd;             // 0x18
    int   initializing;     // 0x1c
    int   doubleSize;       // 0x20
    LPDIRECTDRAW      ddraw;    // 0x24
    LPDIRECT3D        d3d;      // 0x28
    LPDIRECT3DRM      d3drm;    // 0x2c
    LPDIRECTDRAWCLIPPER clipper; // 0x30
    int   depthBitMask;     // 0x34
    RenderFmt fmt;          // 0x38
    int   requestedWidth;   // 0x50
    int   requestedHeight;  // 0x54
    RECT  screenRect;       // 0x58
    int   originX;          // 0x68
    int   originY;          // 0x6c
    int   width;            // 0x70
    int   height;           // 0x74
    int   field_078;        // 0x78
    unsigned char palette[256][4]; // 0x7c
    int   deviceFound;      // 0x47c
    GUID  deviceGuids[16];  // 0x480
    char  deviceNames[16][64]; // 0x580
    int   chosenDevice;     // 0x980
    int   hardware;         // 0x984
    int   monoColorModel;   // 0x988
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
    int   ownsDDObjects;    // 0x9c8

    Renderer();                                 // 0x474400
    void  Close();                              // 0x474520 (the destructor body)
    int   Initialize(void *windowObj, HWND hwnd, int x, int y, int w, int h, int reqW, int reqH, int bpp, int fullscreen, int doubleSize, const unsigned char *rgba);   // 0x474540
    int   Shutdown(int forceRelease);           // 0x474930
    int   SetPaletteEntries(int first, int count, const unsigned char *rgba);   // 0x474c00
    int   UploadFrontPalette();                 // 0x474e10
    void  EnterFullscreenInit();                // 0x474b70
    void  EnterWindowedInit();                  // 0x474bb0
    void  GetClientRectInWindow(LONG out[4]);   // 0x474f70
    int  *GetPixelFormat(int out[6]);           // 0x475000
    void  FadePalette(float seconds);           // 0x4750a0
    int   PrepareForDialog();                   // 0x4753c0 (returns 0)
    void  PutText(HFONT font, int x, int y, const char *text);   // 0x4753d0
    int   Error(const char *fmt, ...);          // 0x4754c0
    int   FatalError(const char *fmt, ...);     // 0x475510
    void  QueryDisplayMode();                   // 0x475560
    void  ApplyQualitySettings();               // 0x4756a0
    void  UpdateViewportPlanes();               // 0x4757d0
    void  UpdateScreenRect();                   // 0x475870
    int   EnumerateDevices();                   // 0x475ad0
    int   CreateFrames();                       // 0x475b40
    int   CreateSurfaces();                     // 0x475c10
    int   CreateViewport();                     // 0x4761f0
    int   EnterDisplayMode(int w, int h);       // 0x476250
    int   SetCooperativeNormalExclusive(LPDIRECTDRAW dd);   // 0x4763c0
    int   SetCooperativeNormal(int unused);     // 0x476410
    int   SetDisplayModeChecked(int w, int h, int bpp);   // 0x476460
    int   PositionWindowed(int w, int h);       // 0x4764c0
    int   PositionFullscreen();                 // 0x4765d0
    void  BuildFrontPaletteEntries(PALETTEENTRY out[256]);
    void  CopyPaletteEntries(int first, int count, const unsigned char *rgba);
    // five more members (private helpers in the original header); the count shifts the compiler's tables and so register choices
    void Pad1();
    void Pad2();
    void Pad3();
    void Pad4();
    void Pad5();
};

typedef unsigned long COLORKEYVAL;
int  CheckDesktopDepth(int *width, int *height);   // 0x4743b0
DWORD __stdcall GetColorKeyValue(LPDIRECTDRAWSURFACE surf, COLORREF color);   // 0x474230
HRESULT __stdcall SetSourceColorKey(LPDIRECTDRAWSURFACE surf, COLORREF color);   // 0x474310
const char *HResultToString(long hr);           // 0x478430
void QueryDesktopMode(int *width, int *height, int *bpp);   // 0x474360
