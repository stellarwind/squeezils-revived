// GIF reader: the game's own minimal GIF decoder (SQZ.EXE 0x479290-0x479b70, entry point 0x479830).
// It only understands what the game's art uses: a global colour table, then one image, no extension blocks, not interlaced.
// The local colour table and interlace flags of the image descriptor are ignored.
#pragma once

// Decodes the first image of a GIF file into one byte per pixel (palette indices, rows top to bottom, no padding).
//   palette: 768 bytes, filled as three planes of 256: red, green, blue (only the entries the file has)
//   width, height: the image size
// Returns the pixels in memory from the game's heap (the caller frees them with Lib_Free), or NULL when the file is missing
// or not a GIF with a global colour table. A damaged image still returns the part that decoded (the rest is zero).
unsigned char *GifDecode(const char *path, unsigned char *palette, unsigned *width, unsigned *height);   // 0x479830
