// The parts of Bitmap (vtable 0x4a1060, size 0x80) the byte-match units use; the slot order is the original's.
// Full behaviour: Repo/src/Bitmap.h. Only declarations: Bitmap itself is not reconstructed in this dialect yet (0x473760-0x474230).
#pragma once

struct IDirectDrawSurface;
struct Renderer { char pad[0x994]; IDirectDrawSurface *primary; };

class Bitmap {
public:
    Bitmap(Renderer *r);                                                                               // 0x473760
    ~Bitmap();                                                                                         // 0x473890
    virtual int CopyRectFrom(Bitmap *src, int a, int b, int c, int d, int e, int f, int g, int h);   // slot 0
    virtual int CopyFrom(Bitmap *src);                                                                // slot 1
    virtual int DrawToBitmap(Bitmap *dest, int x, int y, int sx, int sy);                             // slot 2
    virtual int DrawToSurface(IDirectDrawSurface *dest, int a, int b, int c, int d, int e, int f, int g, int h);   // slot 3
    virtual int DrawToSurfaceAt(IDirectDrawSurface *dest, int x, int y, int sx, int sy, int key);     // slot 4
    virtual IDirectDrawSurface *CreateSurfaceSized(int w, int h);                                     // slot 5
    virtual IDirectDrawSurface *CreateSurface();                                                      // slot 6
    IDirectDrawSurface *GetSurface();                                                                  // 0x4738b0
    int GetWidth();                                                                                    // 0x4739b0
    int GetHeight();                                                                                   // 0x4739c0
    IDirectDrawSurface *Load(const char *path);                                                        // 0x4739d0
    int CaptureFrom(IDirectDrawSurface *src, int x, int y);                                            // 0x473e80
    char pad[0x6c];
    Renderer *renderer;
    IDirectDrawSurface *surface;
    int width;
    int height;
};
