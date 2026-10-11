// Renderer::UpdateViewportPlanes (0x4757d0): front/back planes and a symmetric frustum from the vertical field of view.
#include <math.h>

struct Viewport3D {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13();
    virtual long __stdcall SetFront(float z);                                      // slot 14, +0x38
    virtual long __stdcall SetBack(float z);                                       // slot 15, +0x3c
    virtual void s16();
    virtual long __stdcall SetUniformScaling(int on);                              // slot 17, +0x44
    virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21();
    virtual void s22(); virtual void s23();
    virtual long __stdcall SetPlane(float l, float r, float b, float t);           // slot 24, +0x60
};

struct RendererVp {
    char pad0[0x70];
    int width;                  // +0x70
    int height;                 // +0x74
    char pad1[0x9b8 - 0x78];
    Viewport3D *viewport;       // +0x9b8
    float nearPlane;            // +0x9bc
    float farPlane;             // +0x9c0
    float fovDegrees;           // +0x9c4
    void UpdateViewportPlanes();
};

void RendererVp::UpdateViewportPlanes()
{
    viewport->SetFront(nearPlane);
    viewport->SetBack(farPlane);
    double half = tan(fovDegrees * 0.5f * 0.017453292519943);
    double w = half * nearPlane;
    double h = ((double)height / (double)width) * w;
    viewport->SetPlane((float)-w, (float)w, (float)-h, (float)h);
    viewport->SetUniformScaling(0);
}
