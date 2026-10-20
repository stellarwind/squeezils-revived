// Warp: the game's sky (SQZ.EXE 0x478c80-0x479290), byte-match build. Layout and notes: Repo/legacy_gcc/Warp.h.
#pragma once

struct WarpFace {            // 0x48 bytes
    int   extentA, extentB;
    float zero08, zero0c, one, halfA, halfB, halfA2, k20, zero24, k28;
    float minusOne[6];
    char  flag;
    char  pad[3];
};

// the software sky renderer (free functions at 0x48bb30..0x48d860, not reconstructed here)
void SkyShutdown();                                       // 0x48bba0
unsigned char SkyInit(int h, int w, int unused);                   // 0x48bb30
void SkySetMode(char flag);                               // 0x48c870
void SkySetOrigin(float x, float y, float z);             // 0x48c620
void SkySetFov(float fovX, float fovY);                   // 0x48c680
void SkySetFace(void *data, WarpFace *face);              // 0x48bbc0
void SkySetOutput(void *bits, int depth);                 // 0x48bd30
void SkyResize(int h, int w);                             // 0x48c830
void SkySetView(float *m3);                               // 0x48bd60
void RotationMatrix(float *out, int axis, float angle);   // 0x48d7a0
void MatrixMultiply(float *out, float *a, float *b);      // 0x48d860
void Lib_Free(void *p);                                   // 0x490540

class Warp {
public:
    Warp();                                               // 0x478c80
    ~Warp();                                              // 0x478cb0
    int  Init(int h, int w, float fov, char flag);        // 0x478ce0
    void SetOutput(void *bits);                           // 0x478da0
    void Resize(int w, int h);                            // 0x478dc0
    void Update(const float *m);                          // 0x478e10
    void UpdateCube(const float *m);                      // 0x478e90
    int  Load(const char *prefix, unsigned char *palette, const int *pixelFormat);   // 0x479090
    void UpdateSingle(const float *m);                    // 0x478e30
    int  SelectFace(const float *m3);                     // 0x478fd0

    int      height;        // 0x00
    int      width;         // 0x04
    float    fovX;          // 0x08
    float    fovY;          // 0x0c
    int      current;       // 0x10
    int      faceCount;     // 0x14
    int      outputDepth;   // 0x18
    WarpFace faces[6];      // 0x1c
    void    *faceData[6];   // 0x1cc
};


int ImageLoad(const char *path, const int *fmt, void *(*alloc)(unsigned), int *outWidth, int *outHeight, void **pixels, unsigned char *palette);   // 0x477cc0
