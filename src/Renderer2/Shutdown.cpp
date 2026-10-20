// Renderer::Shutdown (0x474930).
#include "Renderer2/Renderer2.h"

struct WindowListeners { void Remove(void *callback, void *context); };     // 0x477f60 (ListenerTable::Remove)
extern void WindowCallback();                                                // 0x476630

int Renderer::Shutdown(int forceRelease)
{
    if (viewport) {
        viewport->Release();
        viewport = 0;
    }
    if (frontPalette && (ownsDDObjects || forceRelease)) {
        frontPalette->Release();
        frontPalette = 0;
    }
    if (backPalette) {
        backPalette->Release();
        backPalette = 0;
    }
    if (primary && (ownsDDObjects || forceRelease)) {
        primary->Release();
        primary = 0;
    }
    if (backBuffer) {
        backBuffer->Release();
        backBuffer = 0;
    }
    if (depth) {
        depth->Release();
        depth = 0;
    }
    if (device) {
        device->Release();
        device = 0;
    }
    if (ownsDDObjects) {
        if (cameraFrame) {
            cameraFrame->Release();
            cameraFrame = 0;
        }
        if (ownsDDObjects) {
            if (sceneFrame) {
                sceneFrame->Release();
                sceneFrame = 0;
            }
        }
    }
    hardware = 0;
    deviceFound = 0;
    monoColorModel = 1;
    chosenDevice = -1;
    if (ownsDDObjects && fullscreen) {
        ddraw->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
        ddraw->RestoreDisplayMode();
        fullscreen = 0;
    }
    if (clipper) {
        clipper->Release();
        clipper = 0;
    }
    if (ownsDDObjects) {
        if (d3drm) {
            d3drm->Release();
            d3drm = 0;
        }
    }
    if (ownsDDObjects) {
        if (d3d) {
            d3d->Release();
            d3d = 0;
        }
    }
    if (ownsDDObjects) {
        if (ddraw) {
            ddraw->Release();
            ddraw = 0;
        }
    }
    originX = 0;
    originY = 0;
    width = 0;
    height = 0;
    depthBitMask = 0;
    SetRect(&screenRect, 0, 0, 0, 0);
    doubleSize = 0;
    hwnd = 0;
    if (ownsDDObjects && windowObj) {
        ((WindowListeners *)windowObj)->Remove(WindowCallback, this);
        windowObj = 0;
    }
    return 1;
}

void Renderer::EnterFullscreenInit()
{
    int h = doubleSize ? height * 2 : height;
    int w = doubleSize ? width * 2 : width;
    Initialize(windowObj, hwnd, 0, 0, w, h, requestedWidth, requestedHeight, fmt.bpp, 1, doubleSize, 0);
}

void Renderer::EnterWindowedInit()
{
    int h = doubleSize ? height * 2 : height;
    int w = doubleSize ? width * 2 : width;
    Initialize(windowObj, hwnd, originX, originY, w, h, requestedWidth, requestedHeight, fmt.bpp, 0, doubleSize, 0);
}
