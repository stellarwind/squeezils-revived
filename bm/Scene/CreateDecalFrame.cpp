// 0x477400: configure a decal texture and attach it to a new frame.
#include "Scene/Scene.h"
#include "World/WorldDecal.h"

LPDIRECT3DRMFRAME Scene::CreateDecalFrame(LPDIRECT3DRMTEXTURE texture,
    WorldVector position, WorldDecalSettings settings, LPDIRECT3DRMFRAME parent)
{
    D3DRMIMAGE *image = texture->GetImage();
    LPDIRECT3DRMTEXTURE decal;
    renderer->d3drm->CreateTexture(image, &decal);
    int originX = (int)(image->width * settings.origin.x);
    int originY = (int)(image->height * settings.origin.y);
    decal->SetDecalScale(1);
    decal->SetDecalTransparency(settings.useTransparency);
    decal->SetDecalTransparentColor(D3DRMCreateColorRGB(settings.transparentColor.x,
        settings.transparentColor.y, settings.transparentColor.z));
    decal->SetDecalOrigin(originX, originY);
    decal->SetDecalSize(settings.size.x, settings.size.y);
    LPDIRECT3DRMFRAME frame;
    if (parent)
        frame = CreateFrameUnder(parent);
    else
        frame = CreateSceneFrame();
    frame->AddVisual(decal);
    if (parent)
        frame->SetPosition(parent, position.x, position.y, position.z);
    decal->Release();
    return frame;
}
