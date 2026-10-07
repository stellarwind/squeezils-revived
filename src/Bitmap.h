// Bitmap: a 2D image held in a DirectDraw offscreen surface (system memory), used for the game's flat artwork (menus, HUD,
// backdrops) and drawn with colour keyed blits. "Bitmap" is our name (no RTTI in the binary). Reconstructed from SQZ.EXE
// 0x473760-0x474230. Size 0x80.
//
// The class has virtual methods (vtable at 0x4a1060) and the game's own code calls them through it, so the layout and the slot
// numbers below are the original's. Objects are created by the original code; here the vtable pointer is a plain first field
// and virtual calls on `this` and on other bitmaps go through it (the slots can be overridden by a subclass).
#pragma once
#include "Renderer.h"

class Bitmap {
public:
    // virtual slots
    enum { SLOT_COPY_RECT_FROM = 0, SLOT_COPY_FROM = 1, SLOT_DRAW_TO_BITMAP = 2, SLOT_DRAW_TO_SURFACE = 3,
           SLOT_DRAW_TO_SURFACE_AT = 4, SLOT_CREATE_SIZED = 5, SLOT_CREATE = 6 };

    explicit Bitmap(Renderer *r);                                   // 0x473760
    ~Bitmap();                                                      // 0x473890 releases the surface (the pointer is left as it is)

    LPDIRECTDRAWSURFACE GetSurface() const { return surface; }      // 0x4738b0
    int GetWidth() const { return width; }                          // 0x4739b0
    int GetHeight() const { return height; }                        // 0x4739c0

    // virtual
    BOOL CopyRectFrom(Bitmap *src, int srcL, int srcT, int srcR, int srcB, int dstL, int dstT, int dstR, int dstB);   // 0x473810 slot 0
    BOOL CopyFrom(Bitmap *src);                                     // 0x473780 slot 1: same size, keeps the colour key
    BOOL DrawToBitmap(Bitmap *dest, int destX, int destY, int srcX, int srcY);                       // 0x474130 slot 2 (colour keyed)
    BOOL DrawToSurface(LPDIRECTDRAWSURFACE dest, int dstL, int dstT, int dstR, int dstB,
                       int srcL, int srcT, int srcR, int srcB);     // 0x473f70 slot 3 (colour keyed)
    BOOL DrawToSurfaceAt(LPDIRECTDRAWSURFACE dest, int destX, int destY, int srcX, int srcY, int useColorKey);   // 0x474020 slot 4
    LPDIRECTDRAWSURFACE CreateSurfaceSized(int w, int h);           // 0x4738c0 slot 5: a new surface, remembered with its size
    LPDIRECTDRAWSURFACE CreateSurface();                            // 0x473940 slot 6: a new surface of the stored size (not stored)

    // not virtual
    BOOL CaptureFrom(LPDIRECTDRAWSURFACE src, int x, int y);        // 0x473e80 copies the w x h area at (x, y) of src into this
    LPDIRECTDRAWSURFACE Load(const char *path);                     // 0x4739d0 loads an 8 bit .bmp, converts it to the display format

    void **vtbl;                    // 0x00
    DDSURFACEDESC desc;             // 0x04 scratch for CreateSurface
    Renderer *renderer;             // 0x70
    LPDIRECTDRAWSURFACE surface;    // 0x74
    int width;                      // 0x78
    int height;                     // 0x7c

private:
    void ReportError(HRESULT hr) {}                                 // 0x474350: the original does nothing with it
    BOOL BltRetry(LPDIRECTDRAWSURFACE dst, RECT *dstRect, LPDIRECTDRAWSURFACE src, RECT *srcRect, DWORD flags);
    BOOL ReadRetry(HANDLE file, void *buffer, DWORD size, const char *path);
};
