/* GIF reader (SQZ.EXE 0x479290-0x479b10), plain C like the original (it relies on an implicit return, so it is not C++),
 * all state in globals. Byte-match target: see targets.txt. Behaviour notes: Repo/src/Gif.h and Gif.cpp (the GCC version). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static short gBlockLeft = 0;             /* 4c3958  unread bytes in the current sub-block */
static short gBitsLeft = 0;              /* 4c395c  unread bits in gCurByte */
static int gMaskTable[13] = { 0, 1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff };   /* 4c3960 */
static short gEndCode;                   /* 4c8720 */
static unsigned short *gPrefix;          /* 4c8724 */
static char *gSuffix;                    /* 4c8728 */
static char *gStack;                     /* 4c872c */
static unsigned char *gOut;              /* 4c8730 */
static short gFirstFree;                 /* 4c8734 */
static unsigned char gBlock[256];        /* 4c8738 */
static unsigned int gHeight;             /* 4c883c */
static unsigned char *gBlockPtr;         /* 4c8840 */
static short gLimit;                     /* 4c8848 */
static unsigned char gCurByte;           /* 4c884c */
static short gClearCode;                 /* 4c8850 */
static FILE *gFile;                      /* 4c8854 */
static short gCodeSize;                  /* 4c8858 */
static short gNextFree;                  /* 4c885c */
static int gPaletteCount;                /* 4c8860 */
static unsigned char *gRow;              /* 4c8844 */
static int gBadCodes;                    /* 4ed85c */

/* @00479290 Appends count pixels to the image; reports colours outside the palette. */
int EmitRow(src, count)
unsigned char *src;
int count;
{
    for (; count != 0; count--) {
        unsigned int b = *src++;
        *gOut = (unsigned char)b;
        gOut++;
        if (gPaletteCount < (int)b)
            fprintf(stderr, "Error - GIF Image Map Colour out of range\n");
    }
    return 0;
}

/* @004792e0 */
int ReadByte(void)
{
    if (getc(gFile) == -1) {
        fprintf(stderr, "Premature End Of File reading GIF image\n");
        exit(1);
    }
}

/* @00479340 */
void InitCodes(minSize)
short minSize;
{
    gCodeSize = minSize + 1;
    gLimit = 1 << gCodeSize;
    gClearCode = 1 << minSize;
    gEndCode = gClearCode + 1;
    gFirstFree = gEndCode + 1;
    gNextFree = gFirstFree;
    gBitsLeft = 0;
    gBlockLeft = 0;
}

/* @00479390 Next code, least significant bit first. Negative: the file ended. */
short GetCode(void)
{
    short i;
    int c;
    unsigned char first;
    unsigned int code;
    unsigned char *p;

    if (gBitsLeft == 0) {
        if (gBlockLeft <= 0) {
            gBlockPtr = gBlock;
            gBlockLeft = (short)ReadByte();
            if (gBlockLeft < 0)
                return gBlockLeft;
            if (gBlockLeft != 0) {
                i = 0;
                if (0 < gBlockLeft) {
                    do {
                        c = ReadByte();
                        if ((short)c < 0)
                            return c;
                        gBlock[i++] = (char)c;
                    } while (i < gBlockLeft);
                }
            }
        }
        gCurByte = *gBlockPtr++;
        gBlockLeft--;
        gBitsLeft = 8;
    }
    first = gCurByte >> (8 - gBitsLeft);
    code = first;
    if (gBitsLeft < gCodeSize) {
        do {
            if (gBlockLeft <= 0) {
                gBlockPtr = gBlock;
                gBlockLeft = (short)ReadByte();
                if (gBlockLeft < 0)
                    return gBlockLeft;
                if (gBlockLeft != 0) {
                    i = 0;
                    if (0 < gBlockLeft) {
                        do {
                            c = ReadByte();
                            if ((short)c < 0)
                                return c;
                            gBlock[i++] = (char)c;
                        } while (i < gBlockLeft);
                    }
                }
            }
            gCurByte = *gBlockPtr++;
            code |= (unsigned int)gCurByte << gBitsLeft;
            gBlockLeft--;
            gBitsLeft += 8;
        } while (gBitsLeft < gCodeSize);
    }
    gBitsLeft -= gCodeSize;
    code &= gMaskTable[gCodeSize];
    return (short)code;
}

/* @00479520 */
void FreeTables(void)
{
    free(gStack);
    free(gSuffix);
    free(gPrefix);
}

/* @00479550 The image data: minimum code size, then the LZW codes. rowWidth pixels are gathered per row before EmitRow. */
short Decode(rowWidth)
short rowWidth;
{
    short minSize;
    short rowLeft;
    short rc;
    short code;
    int oldCode;
    short firstChar;
    unsigned short in;
    unsigned char *row, *rp, *sp;

    minSize = (short)ReadByte();
    if (minSize < 0)
        return minSize;
    if (minSize < 2 || minSize > 9)
        return -20;
    InitCodes(minSize);
    gStack = (char *)malloc(0x1000);
    gSuffix = (char *)malloc(0x1000);
    gPrefix = (unsigned short *)malloc(0x2000);
    row = gRow;
    sp = (unsigned char *)gStack;
    rp = row;
    oldCode = 0;
    firstChar = 0;
    gBadCodes = 0;
    rowLeft = rowWidth;
    for (;;) {
        code = GetCode();
        if (code == gEndCode)
            break;
        if (code < 0) {
            FreeTables();
            return 0;
        }
        if (code == gClearCode) {
            gCodeSize = minSize + 1;
            gNextFree = gFirstFree;
            gLimit = 1 << gCodeSize;
            do {
                code = GetCode();
            } while (code == gClearCode);
            if (code == gEndCode)
                break;
            if (gNextFree <= code)
                code = 0;
            firstChar = code;
            *rp++ = (unsigned char)code;
            oldCode = code;
            if (--rowLeft == 0) {
                rc = (short)EmitRow(row, rowWidth);
                if (rc < 0) {
                    FreeTables();
                    return rc;
                }
                rp = row;
                rowLeft = rowWidth;
            }
        } else {
            in = code;
            if (gNextFree <= (short)code) {
                if (gNextFree < code)
                    gBadCodes++;
                in = oldCode;
                *sp++ = (unsigned char)firstChar;
            }
            while (gFirstFree <= (short)in) {
                *sp++ = gSuffix[(short)in];
                in = gPrefix[(short)in];
            }
            *sp++ = (unsigned char)in;
            if (gNextFree < gLimit) {
                gSuffix[gNextFree] = (char)in;
                firstChar = in;
                gPrefix[gNextFree] = oldCode;
                gNextFree++;
                oldCode = code;
            }
            if (gLimit <= gNextFree && gCodeSize < 12) {
                gLimit <<= 1;
                gCodeSize++;
            }
            while (sp > (unsigned char *)gStack) {
                *rp++ = *--sp;
                if (--rowLeft == 0) {
                    rc = (short)EmitRow(row, rowWidth);
                    if (rc < 0) {
                        FreeTables();
                        return rc;
                    }
                    rowLeft = rowWidth;
                    rp = row;
                }
            }
        }
    }
    rc = 0;
    if (rowLeft != rowWidth)
        rc = (short)EmitRow(row, rowWidth - rowLeft);
    FreeTables();
    return rc;
}

/* @00479830 Reads the first image of a GIF file; returns the pixel buffer or 0. */
int GifDecode(path, palette, width, height)
char *path;
unsigned char *palette;
unsigned int *width;
unsigned int *height;
{
    int rc;
    int c, done, pixels, i;
    unsigned int w, h;
    unsigned char head[13];

    rc = 0;
    gFile = fopen(path, "rb");
    if (gFile == 0)
        return 0;
    gRow = (unsigned char *)malloc(0x801);
    if (gRow == 0) {
        fclose(gFile);
        return 0;
    }
    for (i = 0; i < 0x801; i++)
        gRow[i] = 0;
    for (i = 0; i < 13; i++)
        head[i] = (unsigned char)ReadByte();
    if (strncmp((char *)head, "GIF", 3) != 0 || head[3] < '0' || head[3] > '9' ||
        head[4] < '0' || head[4] > '9' || head[5] < 'A' || head[5] > 'z') {
        fclose(gFile);
        free(gRow);
        return 0;
    }
    gPaletteCount = 1 << ((head[10] & 0xf) + 1);
    if ((head[10] & 0x80) == 0) {
        fclose(gFile);
        free(gRow);
        return 0;
    }
    for (i = 0; i < gPaletteCount; i++) {
        palette[i] = (unsigned char)ReadByte();
        palette[0x100 + i] = (unsigned char)ReadByte();
        palette[0x200 + i] = (unsigned char)ReadByte();
    }
    done = 0;
    w = *(unsigned int *)head;
    do {
        c = ReadByte();
        switch (c) {
        case 0x21:
            ReadByte();
            while ((c = ReadByte()) > 0) {
                int n = c;
                do {
                    ReadByte();
                    n--;
                } while (n != 0);
            }
            break;
        case 0x2c:
            i = 0;
            do {
                c = ReadByte();
                if (c < 0) {
                    rc = -1;
                    break;
                }
                head[i] = (unsigned char)c;
                i++;
            } while (i < 9);
            if (rc < 0) {
                done = 1;
            } else {
                w = (head[5] << 8) | head[4];
                h = (head[7] << 8) | head[6];
                gHeight = w;
                gOut = (unsigned char *)malloc(h * w);
                pixels = (int)gOut;
                if (gOut == 0) {
                    free(gRow);
                    fclose(gFile);
                    return 0;
                }
                rc = Decode(w);
                done = 1;
            }
            break;
        case 0x3b:
            done = 1;
            rc = 0;
            break;
        default:
            rc = -1;
            done = 1;
            break;
        }
        free(gRow);
        fclose(gFile);
        if (done) {
            *width = w;
            *height = h;
            return pixels;
        }
    } while (1);
}
