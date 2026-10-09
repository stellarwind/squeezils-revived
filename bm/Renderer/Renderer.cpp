// Renderer::GetPixelFormat (0x475000 / 0x475020).
#include <string.h>
#include "Renderer/Renderer.h"

int *Renderer::GetPixelFormat(int out[6])
{
    memcpy(out, format, 24);
    return out;
}

void Renderer::SetShades(int s)
{
    shades = s;
    Rebuild();
}

void Renderer::SetTextureColors(int c)
{
    textureColors = c;
    Rebuild();
}

void Renderer::SetTextureShades(int s)
{
    textureShades = s;
    Rebuild();
}

void Renderer::SetViewParams(float nearP, float farP, float fov)
{
    nearPlane = nearP;
    farPlane = farP;
    fovDegrees = fov;
    Rebuild();
}

void Renderer::Rebuild()
{
}
