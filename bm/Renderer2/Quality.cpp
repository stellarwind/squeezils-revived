// Renderer::ApplyQualitySettings (0x4756a0).
#include "Renderer2/Renderer2.h"

void Renderer::ApplyQualitySettings()
{
    HRESULT hr;
    if (FAILED(hr = d3drm->SetDefaultTextureColors(textureColors)))
        Error("SetDefaultTextureColors(%d) failed.\n%s", textureColors, HResultToString(hr));
    if (FAILED(hr = d3drm->SetDefaultTextureShades(textureShades)))
        Error("SetDefaultTextureShades(%d) failed.\n%s", textureShades, HResultToString(hr));
    if (FAILED(hr = device->SetShades(shades)))
        Error("SetShades(%d) failed.\n%s", shades, HResultToString(hr));
    if (FAILED(hr = device->SetDither(FALSE)))
        Error("SetDither failed (false).\n%s", HResultToString(hr));
    if (FAILED(hr = device->SetQuality(D3DRMRENDER_GOURAUD)))
        Error("SetQuality failed (gouraud).\n%s", HResultToString(hr));
    if (FAILED(hr = device->SetTextureQuality(hardware != 0 ? D3DRMTEXTURE_LINEAR : D3DRMTEXTURE_NEAREST)))
        Error("SetTextureQuality failed (nearest).\n%s", HResultToString(hr));
}
