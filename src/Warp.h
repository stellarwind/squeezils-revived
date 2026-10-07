// Warp: the game's sky. A "six-view warp gif" (cube map, six GIFs named nz ny pz nx px py) is warped into the
// current view by a small software renderer (SkyRenderer, see Sky.h) and written
// into an offscreen surface that Scene stretches onto the back buffer.
// Reconstructed from SQZ.EXE 0x478c80-0x479090. "Warp" is our name (no RTTI). Size 0x1e4, field offsets are the original ones.
// The software renderer only supports 8 and 16 bit output, so the sky is black on 32 bit displays.
#pragma once

struct WarpFace {            // 0x48 bytes, handed to SkyRenderer::SetFace as is. The image is extentB wide, extentA tall;
                             // a fisheye lens circle sits in it: centre (halfA, halfB), radius halfA2, curve k20/zero24/k28
    int   extentA;           // 0x00 image height (second size value returned by the image loader)
    int   extentB;           // 0x04 image width (first size value returned by the image loader)
    float zero08, zero0c;    // 0x08, 0x0c  0   (with one: the face axis, 0 0 1)
    float one;               // 0x10        1.0
    float halfA, halfB;      // 0x14, 0x18  extentA / 2, extentB / 2
    float halfA2;            // 0x1c        extentA / 2 again
    float k20;               // 0x20        0x3f1c71c5 (about 0.6111)
    float zero24;            // 0x24        0
    float k28;               // 0x28        0x3fd1743f (about 1.6364)
    float minusOne[6];       // 0x2c        -1.0 each
    char  flag;              // 0x44        0
    char  pad[3];
};

class Warp {
public:
    enum { FACES = 6 };

    Warp();                                               // 0x478c80
    ~Warp();                                              // 0x478cb0 (shuts the renderer down, frees the face images)
    int  Init(int height, int width, float fov, char flag);   // 0x478ce0 (thiscall, 4 stack args). Returns 1 on success
    void SetOutput(void *bits);                           // 0x478da0 where the next frame is drawn, with the pixel depth of the display
    void Resize(int width, int height);                   // 0x478dc0 (note: Init takes height first, this takes width first)
    void Update(const float *m);                          // 0x478e10 draws the sky for the 4x4 camera matrix m
    int  Load(const char *prefix, unsigned char *palette, const int *pixelFormat);   // 0x479090 loads <prefix>nz.gif ... py.gif

    int      height;        // 0x00 (Scene passes warpHeight first)
    int      width;         // 0x04
    float    fovX;          // 0x08 degrees, default 90
    float    fovY;          // 0x0c default 60, set to height / width * fovX
    int      current;       // 0x10 index of the face last given to the renderer
    int      faceCount;     // 0x14 6 after a successful Load, 0 before
    int      outputDepth;   // 0x18 bits per pixel of the display (taken from pixelFormat[0])
    WarpFace faces[FACES];  // 0x1c
    void    *faceData[FACES];   // 0x1cc decoded pixel data of each face (freed in the destructor)

private:
    void UpdateSingle(const float *m);                    // 0x478e30
    void UpdateCube(const float *m);                      // 0x478e90
    static int SelectFace(const float *m3);               // 0x478fd0 which cube face the view looks at (0..5)
};
