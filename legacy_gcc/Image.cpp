// Image loading: see Image.h. Reconstructed from SQZ.EXE 0x477a90-0x477e5f.
#include "Image.h"
#include "Gif.h"
#include "Original.h"
#include <string.h>


// number of zero bits below the lowest set bit (the original loops forever for an empty mask)
static int LowZeroBits(unsigned mask)
{
    int n = 0;
    for (; mask && (mask & 1) == 0; mask >>= 1)
        n++;
    return n;
}

// 255 / (largest value a channel can hold). An empty mask gives 1 (the original never returns from that case); a channel
// wider than 8 bits still divides by zero, as in the original.
static unsigned ChannelDivisor(unsigned mask, int shift)
{
    unsigned max = mask >> shift;
    return max ? 255 / max : 1;
}

// @00477a90
void ImageConvertRow(void *dst, const DisplayFormat *fmt, const unsigned char *src, const unsigned char *palette, unsigned width)
{
    if (fmt->isPalettized != 0) {
        memcpy(dst, src, width);
        return;
    }
    int bpp = fmt->bpp;
    if (bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32)
        return;

    int shiftR = LowZeroBits(fmt->redMask), shiftG = LowZeroBits(fmt->greenMask), shiftB = LowZeroBits(fmt->blueMask);
    unsigned divR = ChannelDivisor(fmt->redMask, shiftR);
    unsigned divG = ChannelDivisor(fmt->greenMask, shiftG);
    unsigned divB = ChannelDivisor(fmt->blueMask, shiftB);

    unsigned char *out = (unsigned char *)dst;
    for (unsigned i = 0; i < width; i++) {
        const unsigned char *c = palette + src[i];
        unsigned r = c[0] / divR;
        unsigned g = c[0x100] / divG;
        unsigned b = c[0x200] / divB;
        switch (bpp) {
        case 8:
            *out++ = (unsigned char)((b << shiftB) | (g << shiftG) | (r << shiftR));
            break;
        case 16:
            *(unsigned short *)out = (unsigned short)((b << shiftB) | (g << shiftG) | (r << shiftR));
            out += 2;
            break;
        case 24:                                   // written in this fixed order, whatever the masks are
            out[0] = (unsigned char)(r << shiftR);
            out[1] = (unsigned char)(g << shiftG);
            out[2] = (unsigned char)(b << shiftB);
            out += 3;
            break;
        case 32:
            *(unsigned *)out = (b << shiftB) | (g << shiftG) | (r << shiftR);
            out += 4;
            break;
        }
    }
}

// @00477cc0
int ImageLoad(const char *path, const DisplayFormat *fmt, ImageAllocFn alloc, int *outWidth, int *outHeight,
              void **pixels, unsigned char *palette)
{
    int bpp = fmt->bpp;
    if (bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32)
        return 0;
    if (fmt->isPalettized != 0 && bpp != 8)
        return 0;

    unsigned char planes[768] = {};                  // (the original copies the unused part of its stack buffer out)
    unsigned width, height;
    unsigned char *src = GifDecode(path, planes, &width, &height);
    if (!src)
        return 0;

    if (!alloc)
        alloc = Lib_Malloc;
    unsigned stride = ((bpp / 8) * width + 3) & ~3u;
    unsigned char *dst = (unsigned char *)alloc((size_t)height * stride);
    if (!dst) {
        Lib_Free(src);                               // (the original leaks it)
        return 0;
    }
    unsigned char *row = dst;
    for (unsigned y = 0; y < height; y++) {
        ImageConvertRow(row, fmt, src + (size_t)width * y, planes, width);
        row += stride;
    }
    Lib_Free(src);

    *outWidth = (int)width;
    *outHeight = (int)height;
    *pixels = dst;
    memcpy(palette, planes, sizeof planes);
    return 1;
}
