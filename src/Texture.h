// Texture creation: a GIF file becomes a D3DRM texture. The game decodes the GIF itself (image loader at 0x477cc0, which
// converts to the display's pixel format), wraps the pixels and palette in a D3DRMIMAGE and hands that to
// IDirect3DRM::CreateTexture. Reconstructed from SQZ.EXE 0x4153f0-0x4156b9.
#pragma once
#include "Renderer.h"

// path:        the GIF file
// transparent: colour (r, g, b, each 0..1) that becomes palette index 0, or NULL. Only meaningful for palettized images.
// pixelFormat: the display's DisplayFormat (6 ints, bpp first), the loader converts the image to it
// byteCounter: running total of texture memory in use; this texture's bytes are added now and subtracted when it is destroyed
// Returns the texture, or NULL when the file cannot be loaded or CreateTexture fails.
// The image buffers belong to the texture and are freed by a destroy callback.
IDirect3DRMTexture *CreateTextureFromGif(const char *path, const float *transparent, const int *pixelFormat, int *byteCounter);   // 0x415440
