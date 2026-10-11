// Bitmap (vtable 0x4a1060, size 0x80) for the byte-match build: SQZ.EXE 0x473700-0x474230.
#pragma once
#include "Renderer2/Renderer2.h"

class Bitmap {
public:
    Bitmap(Renderer *r);                                                                                 // 0x473760
    ~Bitmap();                                                                                           // 0x473890
    virtual int CopyRectFrom(Bitmap *src, RECT srcRect, RECT dstRect);     // slot 0, 0x473810
    virtual int CopyFrom(Bitmap *src);                                                                  // slot 1, 0x473780
    virtual int DrawToBitmap(Bitmap *dest, int x, int y, int sx, int sy);                               // slot 2, 0x474130
    virtual int DrawToSurface(LPDIRECTDRAWSURFACE dest, RECT dstRect, RECT srcRect);   // slot 3, 0x473f70
    virtual int DrawToSurfaceAt(LPDIRECTDRAWSURFACE dest, int x, int y, int sx, int sy, int key);       // slot 4, 0x474020
    virtual LPDIRECTDRAWSURFACE CreateSurfaceSized(int w, int h);                                       // slot 5, 0x4738c0
    virtual LPDIRECTDRAWSURFACE CreateSurface();                                                        // slot 6, 0x473940
    LPDIRECTDRAWSURFACE GetSurface();                                                                    // 0x4738b0
    int GetWidth();                                                                                      // 0x4739b0
    int GetHeight();                                                                                     // 0x4739c0
    int CaptureFrom(LPDIRECTDRAWSURFACE src, int x, int y);                                              // 0x473e80
    void ReportError(long hr);                                                                           // 0x474350
    LPDIRECTDRAWSURFACE Load(const char *path);                                                          // 0x4739d0
    long SetColorKey(LPDIRECTDRAWSURFACE surf, COLORREF color);                                          // 0x474310
    DDSURFACEDESC desc;          // +4
    Renderer *renderer;          // +0x70
    LPDIRECTDRAWSURFACE surface; // +0x74
    int width;                   // +0x78
    int height;                  // +0x7c
};
