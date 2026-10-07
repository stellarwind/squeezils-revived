// SkyRenderer: the software renderer behind the sky (Warp). Reconstructed from SQZ.EXE 0x48bb30-0x48e2ff.
//
// It is a small stand-alone library (C style, global state) that the original links in: it takes one cube-map face as a
// flat image with a fisheye-like lens (WarpFace: centre, radius and a lens curve) and, for a 3x3 view matrix, produces a
// rows x cols picture of what that face looks like from the camera.
//
// How it works (the "mode 1" path, which is the only one the game uses):
//  * Init/SetFov build, for every output row, three viewing rays (left end, middle, right end of the row).
//  * SetView turns those rays by the view matrix and projects each one back into the face image with the lens curve
//    (Project). That leaves, for every row, three (x, y) texel positions.
//  * Draw fits a parabola through the three positions of each row and walks it in 13.19 fixed point with forward
//    differences, one texel read per output pixel (DrawRow). The output is 8 or 16 bit; there is no 32 bit path.
//
// Not reconstructed: the other two modes (SetMode 0 and 2: a whole-image resampler and a cylindrical projection) and the
// least-squares coefficient table the first of them used. The game always selects mode 1, so none of that runs.
#pragma once
#include <stddef.h>
#include "Warp.h"

class SkyRenderer {
public:
    enum { MAX_ROWS = 1200 };         // the original's tables hold 1200 rows (it never checks)

    unsigned char Init(int rows, int cols, unsigned char yUp);             // 0x48bb30 (0 = ok)
    unsigned char Shutdown();                                              // 0x48bba0 (1 when it was not running)
    unsigned char SetFace(const void *pixels, const WarpFace *face);       // 0x48bbc0 (0 ok, 1 not initialised, 2 bad lens, 3 face too big)
    unsigned char SetOutput(void *bits, int depth);                        // 0x48bd30 (only 8 and 16 bit)
    unsigned char SetView(const float *m3);                                // 0x48bd60 (0 ok, 1 not initialised, 2 view not covered by the face, 3 not a rotation)
    unsigned char SetOrigin(float x, float y, float z);                    // 0x48c620 view from three euler angles in degrees
    unsigned char SetFov(float fovX, float fovY);                          // 0x48c680 degrees, each in (0, 180)
    unsigned char Draw();                                                  // 0x48c710 renders into the output (4 when there is none)
    unsigned char Resize(int rows, int cols);                              // 0x48c830
    unsigned char SetMode(char mode);                                      // 0x48c870 0, 1 or 2 (only 1 is implemented)

    static void RotationMatrix(float *out, int axis, float angle);         // 0x48d7a0 rotation about axis 0, 1 or 2, row major 3x3
    static void MatrixMultiply(float *out, const float *a, const float *b);// 0x48d860 out = a * b (3x3)

    // state, original addresses in comments. Public so the hook can compare it with the original's.
    bool           initialized = false;     // 0x4c4e3c
    int            rows = 0;                // 0x4ed960 output height
    int            cols = 0;                // 0x4ed968 output width
    int            colsSquared = 0;         // 0x4ed96c
    float          fovX = 90.0f;            // 0x4cd4f8 degrees
    float          fovY = 60.0f;            // 0x4cd4fc
    unsigned char  yUp = 0;                 // 0x4c897c 0: the rows run downwards (what the game passes)
    char           mode = 0;                // 0x4c4e48
    const unsigned char *face = NULL;       // 0x4c4e40 face pixels (8 or 16 bit, like the output)
    int            faceHeight = 0;          // 0x4ed864
    int            faceWidth = 0;           // 0x4ed860 also the row stride of the face
    float          centerX = 0, centerY = 0, radius = 0;     // 0x4ed95c 0x4ed964 0x4ed894 the lens circle in the face
    float          lensLimit = 0.5f;        // 0x4c4e4c   angle (as a fraction of pi) from which the lens reaches its radius
    float          lensA = 0.0f;            // 0x4c4e50   lens curve r = radius * (lensA * t + lensB) * t, t = angle / pi
    float          lensB = 2.0f;            // 0x4c4e54
    char           wrap = 0;                // 0x4c4e58   the face wraps around horizontally (a panorama); the game's faces do not
    void          *output = NULL;           // 0x4c4e44
    int            depth = 0;               // 0x4ed898 bits per output pixel
    float          faceMatrix[9] = {};      // 0x4ed870 turns the view into the face's axes
    float          faceAxisNeg[3] = {};     // 0x4c8920 the face's viewing axis, negated
    float          view[9] = {};            // 0x4c89a8 the view matrix last accepted by SetView
    float          rays[MAX_ROWS][3][3];    // 0x4d2070 per row: left, middle and right ray (unit vectors)
    float          faceX[MAX_ROWS][4];      // 0x4c89f8 per row: x of the three projected rays in the face image (4th unused)
    float          faceY[MAX_ROWS][4];      // 0x4cd500 per row: y

private:
    void BuildRays();                                                       // 0x48ca50
    void Project(float x, float negY, float z, float *outX, float *outY) const;   // 0x48d050
    template <typename Pixel> void DrawRow(Pixel *dst, float *px, float *py);     // 0x48ddc0 (8 bit) / 0x48e230 (16 bit)
};

extern SkyRenderer g_sky;
