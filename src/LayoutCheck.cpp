// Compile-time check that the reconstructed Renderer layout matches the original offsets.
#include "Renderer.h"
#include <stddef.h>
#define CHECK(f, off) static_assert(offsetof(Renderer, f) == off, "Renderer::" #f " offset")
CHECK(savedWidth, 0x00); CHECK(fullscreen, 0x0c); CHECK(displayState, 0x10); CHECK(windowObj, 0x14);
CHECK(hwnd, 0x18); CHECK(initializing, 0x1c); CHECK(doubleSize, 0x20); CHECK(ddraw, 0x24); CHECK(d3d, 0x28);
CHECK(d3drm, 0x2c); CHECK(clipper, 0x30); CHECK(depthBitMask, 0x34); CHECK(fmt, 0x38); CHECK(requestedWidth, 0x50);
CHECK(screenRect, 0x58); CHECK(originX, 0x68); CHECK(width, 0x70); CHECK(height, 0x74); CHECK(palette, 0x7c);
CHECK(deviceFound, 0x47c); CHECK(deviceGuids, 0x480); CHECK(deviceNames, 0x580); CHECK(chosenDevice, 0x980);
CHECK(hardware, 0x984); CHECK(monoColorModel, 0x988); CHECK(sceneFrame, 0x98c); CHECK(cameraFrame, 0x990);
CHECK(primary, 0x994); CHECK(backBuffer, 0x998); CHECK(depth, 0x99c); CHECK(frontPalette, 0x9a0);
CHECK(backPalette, 0x9a4); CHECK(device, 0x9a8); CHECK(textureColors, 0x9ac); CHECK(viewport, 0x9b8);
CHECK(nearPlane, 0x9bc); CHECK(farPlane, 0x9c0); CHECK(fovDegrees, 0x9c4); CHECK(ownsDDObjects, 0x9c8);
static_assert(sizeof(Renderer) == 0x9cc, "sizeof(Renderer)");
static_assert(D3DRMRENDER_GOURAUD == 0x89, "gouraud constant");
static_assert(sizeof(DDSURFACEDESC) == 0x6c && sizeof(DDCAPS) == 0x13c, "DD struct sizes");
static_assert(offsetof(D3DDEVICEDESC, dwDeviceRenderBitDepth) == 0x9c, "device desc render depth offset");
static_assert(offsetof(D3DDEVICEDESC, dcmColorModel) == 8, "device desc color model offset");
