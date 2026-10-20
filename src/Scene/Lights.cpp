// Scene lights, SQZ.EXE 0x477700 (ambient) and 0x4775c0 (directional). Colour and vectors come by value as the 12-byte helper
// object with the empty destructor at 0x401660 (hence the exception frame).
#include "Scene/Scene.h"

struct V3 {
    float x, y, z;
    V3();
    ~V3();
};

class SceneL {
public:
    Renderer *renderer;
    LPDIRECTDRAWSURFACE warpSurface;
    LPDIRECT3DRMFRAME lightsFrame;
    LPDIRECT3DRMFRAME CreateAmbientLight(V3 rgb);
    LPDIRECT3DRMFRAME CreateDirectionalLight(V3 rgb, V3 pos, V3 dir, V3 up);
};

LPDIRECT3DRMFRAME SceneL::CreateAmbientLight(V3 rgb)
{
    LPDIRECT3DRMFRAME frame;
    LPDIRECT3DRMLIGHT light;
    renderer->d3drm->CreateFrame(lightsFrame, &frame);
    renderer->d3drm->CreateLightRGB(D3DRMLIGHT_AMBIENT, rgb.x, rgb.y, rgb.z, &light);
    frame->AddLight(light);
    light->Release();
    return frame;
}

LPDIRECT3DRMFRAME SceneL::CreateDirectionalLight(V3 rgb, V3 pos, V3 dir, V3 up)
{
    LPDIRECT3DRMFRAME frame;
    LPDIRECT3DRMLIGHT light;
    renderer->d3drm->CreateFrame(lightsFrame, &frame);
    frame->SetPosition(renderer->sceneFrame, pos.x, pos.y, pos.z);
    frame->SetOrientation(renderer->sceneFrame, dir.x, dir.y, dir.z, up.x, up.y, up.z);
    renderer->d3drm->CreateLightRGB(D3DRMLIGHT_DIRECTIONAL, rgb.x, rgb.y, rgb.z, &light);
    frame->AddLight(light);
    light->Release();
    frame->AddRef();
    frame->Release();
    return frame;
}
