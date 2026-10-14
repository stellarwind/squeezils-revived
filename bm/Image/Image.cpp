/* Image loading (SQZ.EXE 0x477a90 ImageConvertRow, 0x477cc0 ImageLoad). Behaviour notes: Repo/src/Image.h and Image.cpp. */
#include <string.h>
#include <stdlib.h>

struct DisplayFormat {
    int bpp;                 /* +0 */
    int isPalettized;        /* +4 */
    unsigned redMask;        /* +8 */
    unsigned greenMask;      /* +c */
    unsigned blueMask;       /* +10 */
};

typedef void *(*ImageAllocFn)(unsigned size);
extern "C" unsigned char *GifDecode(const char *path, unsigned char *palette, int *width, int *height);   /* 0x479830 */

static inline int Shift(unsigned m)
{
    int n = 0;
    while ((m & 1) == 0) { m >>= 1; n++; }
    return n;
}

/* @00477a90 */
void ImageConvertRow(void *dst, const DisplayFormat *fmt, const unsigned char *src, const unsigned char *palette, int width)
{
    if (fmt->isPalettized != 0) {
        memcpy(dst, src, width);
        return;
    }
    int bpp = fmt->bpp;
    if (bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32)
        return;

    int shiftR = Shift(fmt->redMask);
    int divR = 255 / (fmt->redMask >> shiftR);
    int shiftG = Shift(fmt->greenMask);
    int divG = 255 / (fmt->greenMask >> shiftG);
    int shiftB = Shift(fmt->blueMask);
    int divB = 255 / (fmt->blueMask >> shiftB);

    unsigned *p32 = (unsigned *)dst;
    unsigned short *p16 = (unsigned short *)dst;
    unsigned char *p8 = (unsigned char *)dst;
    const unsigned char *s = src;
    int n = width;
    while (n > 0) {
        const unsigned char *c = palette + *s;
        int r = c[0] / divR;
        int g = c[0x100] / divG;
        int b = c[0x200] / divB;
        switch (fmt->bpp) {
        case 8:
            *p8++ = (unsigned char)((b << shiftB) | (g << shiftG) | (r << shiftR));
            break;
        case 16:
            *p16++ = (unsigned short)((b << shiftB) | (g << shiftG) | (r << shiftR));
            break;
        case 24:
            *p8++ = (unsigned char)(r << shiftR);
            *p8++ = (unsigned char)(g << shiftG);
            *p8++ = (unsigned char)(b << shiftB);
            break;
        case 32:
            *p32++ = (b << shiftB) | (g << shiftG) | (r << shiftR);
            break;
        }
        s++;
        n--;
    }
}

/* @00477cc0 */
int ImageLoad(const char *path, const DisplayFormat *fmt, ImageAllocFn alloc, int *outWidth, int *outHeight,
              void **pixels, unsigned char *palette)
{
    int bpp = fmt->bpp;
    if (bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32)
        return 0;
    if (fmt->isPalettized != 0 && bpp != 8)
        return 0;

    unsigned char planes[768];
    int width, height;
    unsigned char *src = GifDecode(path, planes, &width, &height);
    if (!src)
        return 0;

    if (!alloc)
        alloc = (ImageAllocFn)malloc;
    int stride = ((fmt->bpp / 8) * width + 3) & ~3;
    int size = height * stride;
    unsigned char *dst = (unsigned char *)alloc(size);
    if (!dst)
        return 0;
    int y = 0;
    unsigned char *row = dst;
    for (; y < height; y++) {
        unsigned char *s = src + width * y;
        ImageConvertRow(row, fmt, s, planes, width);
        row += stride;
    }
    free(src);
    if (!dst)
        return 0;
    *outWidth = width;
    *outHeight = height;
    *pixels = dst;
    for (int i = 0; i < 256; i++) {
        palette[i] = planes[i];
        palette[i + 256] = planes[i + 256];
        palette[i + 512] = planes[i + 512];
    }
    return 1;
}
