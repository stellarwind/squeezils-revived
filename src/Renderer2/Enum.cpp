// Direct3D device enumeration: the callback (0x475940) and Renderer::EnumerateDevices (0x475ad0).
#include <string.h>
#include "Renderer2/Renderer2.h"

HRESULT __stdcall EnumDeviceCallback(GUID *guid, LPSTR desc, LPSTR name, LPD3DDEVICEDESC hw, LPD3DDEVICEDESC sw, LPVOID ctx)
{
    Renderer *r = (Renderer *)ctx;
    LPD3DDEVICEDESC d = hw;
    if (hw->dcmColorModel == 0)
        d = sw;
    if (!(r->depthBitMask & d->dwDeviceRenderBitDepth))
        return 1;
    r->deviceGuids[r->deviceFound] = *guid;
    strcpy(r->deviceNames[r->deviceFound], name);
    if (r->chosenDevice == -1) {
        r->chosenDevice = r->deviceFound;
        r->hardware = d == hw;
        r->monoColorModel = d->dcmColorModel & 1;
    } else if (d == hw && !r->hardware) {
        r->chosenDevice = r->deviceFound;
        r->hardware = 1;
        r->monoColorModel = d->dcmColorModel & 1;
    } else if (d != hw && r->hardware) {
    } else if (d != hw && r->fmt.isPalettized != 0 && (d->dcmColorModel & 1)) {
        r->chosenDevice = r->deviceFound;
        r->hardware = d == hw;
        r->monoColorModel = 1;
    } else if (!r->fmt.isPalettized) {
        if (d->dcmColorModel & 2) {
            r->chosenDevice = r->deviceFound;
            r->hardware = d == hw;
            r->monoColorModel = 0;
        }
    }
    r->deviceFound++;
    return r->deviceFound != 16;
}

int Renderer::EnumerateDevices()
{
    chosenDevice = -1;
    HRESULT hr = d3d->EnumDevices(EnumDeviceCallback, this);
    if (FAILED(hr)) {
        Error("EnumDevices failed.\n%s", HResultToString(hr));
        return 0;
    }
    if (deviceFound == 0) {
        Error("Can't find a compatible Direct3D driver.");
        return 0;
    }
    return 1;
}
