// Renderer::Renderer (0x474400) and 0x474520 (owns the DirectDraw objects, then Shutdown).
#include "Renderer2/Renderer2.h"

Renderer::Renderer()
{
    initializing = 0;
    windowObj = 0;
    hwnd = 0;
    fullscreen = 0;
    doubleSize = 0;
    ddraw = 0;
    d3d = 0;
    d3drm = 0;
    clipper = 0;
    depthBitMask = 0;
    SetRect(&screenRect, 0, 0, 0, 0);
    originX = 0;
    originY = 0;
    width = 0;
    height = 0;
    for (int i = 0; i < 236; i++) {
        unsigned char v = (unsigned char)(i * 255 / 235);
        palette[10 + i][0] = v;
        palette[10 + i][1] = v;
        palette[10 + i][2] = v;
    }
    deviceFound = 0;
    chosenDevice = -1;
    hardware = 0;
    primary = 0;
    backBuffer = 0;
    depth = 0;
    frontPalette = 0;
    backPalette = 0;
    device = 0;
    textureColors = 8;
    textureShades = 8;
    shades = 8;
    viewport = 0;
    monoColorModel = 1;
    sceneFrame = 0;
    cameraFrame = 0;
    field_078 = 0;
    ownsDDObjects = 0;
    savedWidth = -1;
    savedHeight = -1;
    requestedWidth = 0;
    requestedHeight = 0;
    displayState = 0;
    nearPlane = 1.0f;
    farPlane = 1000.0f;
    fovDegrees = 90.0f;
}

void Renderer::Close()
{
    ownsDDObjects = 1;
    Shutdown(0);
}
