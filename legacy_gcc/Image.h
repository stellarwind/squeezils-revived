// Image loading: a GIF file into pixels in the display's own format (SQZ.EXE 0x477a90 and 0x477cc0).
// Used for the textures (Texture.cpp) and the six sky faces (Warp.cpp).
#pragma once
#include <stddef.h>
#include "Renderer.h"       // DisplayFormat

typedef void *(*ImageAllocFn)(size_t size);

// @00477cc0 Loads `path` and converts it to `fmt` (8, 16, 24 or 32 bit; a palettized 8 bit display keeps the palette indices).
// Each output row is padded to a multiple of 4 bytes. The pixels go into memory from `alloc` (NULL: the game's malloc).
// palette: 768 bytes, red, green and blue planes of 256, the GIF's own palette (used when the display is palettized).
// Returns 1 on success; 0 when the format is not supported, the file cannot be read or memory runs out.
int ImageLoad(const char *path, const DisplayFormat *fmt, ImageAllocFn alloc, int *outWidth, int *outHeight,
              void **pixels, unsigned char *palette);

// @00477a90 One row of palette indices to the display format: each colour channel is scaled down to its mask's range
// (value / (255 / channel max)) and shifted into place. A palettized display just gets the indices.
void ImageConvertRow(void *dst, const DisplayFormat *fmt, const unsigned char *src, const unsigned char *palette, unsigned width);
