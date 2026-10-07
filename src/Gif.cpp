// GIF reader: see Gif.h. Reconstructed from SQZ.EXE 0x479290-0x479a4f (the original keeps all state in globals and reads
// through the C runtime's FILE buffer; here the state lives in a small decoder object and the file through fgetc).
#include "Gif.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

extern void *Lib_Malloc(size_t n);     // @004904c0 the game's malloc
extern void  Lib_Free(void *p);        // @00490540

namespace {

// @004792e0 One byte of the file. Running out of data is fatal in the original: it prints this and exits.
int ReadByte(FILE *file)
{
    int c = fgetc(file);
    if (c == EOF) {
        fprintf(stderr, "Premature End Of File reading GIF image\n");
        exit(1);
    }
    return c;
}

// The LZW decoder for one image (@00479340 start, @00479390 code reader, @00479550 main loop, @00479290 row writer).
class Lzw {
public:
    Lzw(FILE *f, int paletteSize, unsigned char *out, size_t outSize)
        : file(f), paletteCount(paletteSize), outPtr(out), outEnd(out + outSize) {}

    void Run(int rowWidth);

private:
    int  GetCode();                                     // @00479390 next code, least significant bit first
    void ReadBlock();                                   // refills `block` from the next sub-block of the file
    void EmitRow(const unsigned char *row, int count);  // @00479290 appends decoded pixels to the image

    FILE *file;
    int paletteCount;
    unsigned char *outPtr, *outEnd;

    int codeSize = 0;          // bits per code now (min code size + 1, grows to 12)
    int limit = 0;             // 1 << codeSize
    int bitsLeft = 0;          // unread bits in curByte
    int blockLeft = 0;         // unread bytes in the current sub-block
    int curByte = 0;
    unsigned char block[256] = {};
    int blockPos = 0;
};

// @00479390
int Lzw::GetCode()
{
    if (bitsLeft == 0) {
        if (blockLeft < 1)
            ReadBlock();
        curByte = block[blockPos++];
        blockLeft--;
        bitsLeft = 8;
    }
    unsigned code = (unsigned)curByte >> (8 - bitsLeft);
    while (bitsLeft < codeSize) {
        if (blockLeft < 1)
            ReadBlock();
        curByte = block[blockPos++];
        blockLeft--;
        code |= (unsigned)curByte << bitsLeft;
        bitsLeft += 8;
    }
    bitsLeft -= codeSize;
    return code & ((1u << codeSize) - 1);
}

// Sub-blocks are a length byte followed by that many bytes; a length of 0 ends the data. Reading past that end gives the
// stale first byte of the buffer, as in the original (a valid file's end-of-information code comes first).
void Lzw::ReadBlock()
{
    blockPos = 0;
    int count = ReadByte(file);
    blockLeft = count;
    for (int i = 0; i < count; i++)
        block[i] = (unsigned char)ReadByte(file);
}

// @00479290 Pixels beyond the palette size are reported (one message each, as the original) but still stored.
void Lzw::EmitRow(const unsigned char *row, int count)
{
    for (int i = 0; i < count; i++) {
        unsigned char b = row[i];
        if (outPtr < outEnd)             // the original has no bound; a valid file never goes past the image
            *outPtr++ = b;
        if (paletteCount < (int)b)
            fprintf(stderr, "Error - GIF Image Map Colour out of range\n");
    }
}

// @00479550 The image data: a minimum code size byte, then the sub-blocks. rowWidth is the image width; decoded pixels are
// collected into rows of that width.
void Lzw::Run(int rowWidth)
{
    int minSize = ReadByte(file);
    if (minSize < 2 || minSize > 9)
        return;                                          // the original reports -20 and leaves the image untouched

    const int clearCode = 1 << minSize;
    const int endCode = clearCode + 1;
    const int firstFree = clearCode + 2;
    int nextFree = firstFree;
    codeSize = minSize + 1;
    limit = 1 << codeSize;
    bitsLeft = 0;
    blockLeft = 0;

    unsigned short prefix[4096] = {};                    // the table: entry n = prefix[n] followed by suffix[n]
    unsigned char suffix[4096] = {};
    unsigned char stack[4100];                           // a code's pixels come out reversed
    unsigned char *sp = stack;
    std::vector<unsigned char> row(rowWidth);
    unsigned char *rp = row.data();
    int rowLeft = rowWidth;
    int oldCode = 0, firstChar = 0;

    for (;;) {
        int code = GetCode();
        if (code == endCode)
            break;

        if (code == clearCode) {
            codeSize = minSize + 1;
            nextFree = firstFree;
            limit = 1 << codeSize;
            do {
                code = GetCode();
            } while (code == clearCode);
            if (code == endCode)
                break;
            if (nextFree <= code)
                code = 0;
            *rp++ = (unsigned char)code;
            oldCode = firstChar = code;
            if (--rowLeft == 0) {
                EmitRow(row.data(), rowWidth);
                rp = row.data();
                rowLeft = rowWidth;
            }
            continue;
        }

        int c = code;
        if (nextFree <= code) {                          // not in the table yet: the previous string plus its first pixel
            c = oldCode;
            *sp++ = (unsigned char)firstChar;
        }
        int steps = 0;
        while (firstFree <= c && steps++ < 4096) {       // walk the string back to its first pixel (the step limit is ours: it only stops a corrupt table from looping)
            *sp++ = suffix[c];
            c = prefix[c];
        }
        *sp++ = (unsigned char)c;
        if (nextFree < limit) {                          // add oldCode + this string's first pixel to the table
            suffix[nextFree] = (unsigned char)c;
            prefix[nextFree] = (unsigned short)oldCode;
            nextFree++;
            firstChar = c;
            oldCode = code;
        }
        if (limit <= nextFree && codeSize < 12) {
            limit <<= 1;
            codeSize++;
        }
        while (sp > stack) {
            *rp++ = *--sp;
            if (--rowLeft == 0) {
                EmitRow(row.data(), rowWidth);
                rp = row.data();
                rowLeft = rowWidth;
            }
        }
    }
    if (rowLeft != rowWidth)
        EmitRow(row.data(), rowWidth - rowLeft);
}

} // namespace

// @00479830
unsigned char *GifDecode(const char *path, unsigned char *palette, unsigned *width, unsigned *height)
{
    FILE *file = fopen(path, "rb");
    if (!file)
        return NULL;

    // signature "GIF", version digits ("87a", "89a"), logical screen descriptor
    unsigned char head[13];
    for (int i = 0; i < 13; i++)
        head[i] = (unsigned char)ReadByte(file);
    if (strncmp((const char *)head, "GIF", 3) != 0 || head[3] < '0' || head[3] > '9' ||
        head[4] < '0' || head[4] > '9' || head[5] < 'A' || head[5] > 'z') {
        fclose(file);
        return NULL;
    }
    int paletteCount = 1 << ((head[10] & 0xf) + 1);      // (the original masks four bits, not three)
    if (!(head[10] & 0x80)) {                            // there has to be a global colour table
        fclose(file);
        return NULL;
    }
    for (int i = 0; i < paletteCount; i++) {
        unsigned char r = (unsigned char)ReadByte(file);
        unsigned char g = (unsigned char)ReadByte(file);
        unsigned char b = (unsigned char)ReadByte(file);
        if (i < 256) {
            palette[i] = r;
            palette[0x100 + i] = g;
            palette[0x200 + i] = b;
        }
    }

    for (;;) {
        int block = ReadByte(file);
        if (block == 0x21) {                             // extension: label, then sub-blocks until an empty one
            ReadByte(file);                              // (the original closes the file here and then fails on the next
            for (int n = ReadByte(file); n > 0; n = ReadByte(file))   // read, so a file with extensions would end the game)
                while (n-- > 0)
                    ReadByte(file);
        } else if (block == 0x2c) {                      // image descriptor
            unsigned char d[9];
            for (int i = 0; i < 9; i++)
                d[i] = (unsigned char)ReadByte(file);
            unsigned w = d[4] | (d[5] << 8);
            unsigned h = d[6] | (d[7] << 8);
            unsigned char *pixels = (unsigned char *)Lib_Malloc((size_t)w * h);
            if (!pixels) {
                fclose(file);
                return NULL;
            }
            memset(pixels, 0, (size_t)w * h);            // (the original leaves what malloc gave if decoding stops early)
            if (w > 0) {
                Lzw lzw(file, paletteCount, pixels, (size_t)w * h);
                lzw.Run((int)w);
            }
            fclose(file);
            *width = w;
            *height = h;
            return pixels;
        } else {                                         // trailer or junk before any image
            fclose(file);
            return NULL;
        }
    }
}
