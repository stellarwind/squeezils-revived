// Renderer display setup helpers: 0x4763c0 0x476410 0x476460 0x4761f0 0x475b40.
#include "Renderer2/Renderer2.h"

int Renderer::SetCooperativeNormalExclusive(LPDIRECTDRAW dd)
{
    HRESULT hr = dd->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if (FAILED(hr)) {
        Error("SetCooperativeLevel failed.\n%s", HResultToString(hr));
        Shutdown(0);
        return 0;
    }
    return 1;
}

int Renderer::SetCooperativeNormal(int unused)
{
    HRESULT hr = ddraw->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
    if (FAILED(hr)) {
        Error("SetCooperativeLevel failed.\n%s", HResultToString(hr));
        Shutdown(0);
        return 0;
    }
    return 1;
}

int Renderer::SetDisplayModeChecked(int w, int h, int bpp)
{
    HRESULT hr = ddraw->SetDisplayMode(w, h, bpp);
    if (FAILED(hr)) {
        Error("could not set display mode.\n%s", HResultToString(hr));
        Shutdown(0);
        return 0;
    }
    return 1;
}

int Renderer::CreateViewport()
{
    HRESULT hr = d3drm->CreateViewport(device, cameraFrame, 0, 0, width, height, &viewport);
    if (FAILED(hr)) {
        Error("CreateViewport failed.\n%s", HResultToString(hr));
        return 0;
    }
    return 1;
}

int Renderer::CreateFrames()
{
    HRESULT hr;
    if (sceneFrame == 0 && FAILED(hr = d3drm->CreateFrame(0, &sceneFrame))) {
        Error("CreateFrame failed (scene).\n%s", HResultToString(hr));
        return 0;
    }
    if (cameraFrame == 0) {
        if (FAILED(hr = d3drm->CreateFrame(sceneFrame, &cameraFrame))) {
            Error("CreateFrame failed (camera).\n%s", HResultToString(hr));
            return 0;
        }
        if (FAILED(hr = cameraFrame->SetPosition(sceneFrame, 0, 0, 0))) {
            Error("SetPosition failed (camera).\n%s", HResultToString(hr));
            return 0;
        }
    }
    return 1;
}
