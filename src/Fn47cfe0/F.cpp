// 0x47cfe0: NavMesh::ReadBinary - header, 16 bit point count + floats, 16 bit face count + face records, then the box tree.
#include <stdio.h>
#include <string.h>
extern const char *gSurfaceBinaryHeader;                    // 0x4c3e2c
int ReadShort(FILE *f, unsigned short *out);                // 0x47c690
int ReadInt(FILE *f, int *out);                             // 0x47c6f0
int ReadFloat(FILE *f, float *out);                         // 0x47c770
#include "NavMesh/NavMesh.h"
typedef MVert Point;
typedef MFace Face;
int NavMesh::ReadBinary(FILE *f)
{
    char header[128];
    unsigned short number;
    unsigned i;
    int off;
    const char *msg;
    unsigned arg;
    if (fread(header, strlen(gSurfaceBinaryHeader), 1, f) != 1) {
        fprintf(stderr, "ERROR: can't read file header\n");
        return 0;
    }
    if (ReadShort(f, &number) != 1) {
        fprintf(stderr, "ERROR: can't read number of points in surface\n");
        return 0;
    }
    pointCount = number;
    if (!AllocPoints(number)) {
arg = pointCount;        msg = "ERROR: can't allocate %d points for surface\n";        goto fail;
    }
    off = 0;
    for (i = 0; i < pointCount; i++) {
        if (ReadFloat(f, &((Point *)((char *)points + off))->x) != 1 || ReadFloat(f, &((Point *)((char *)points + off))->y) != 1 ||
            ReadFloat(f, &((Point *)((char *)points + off))->z) != 1) {
arg = pointCount;            msg = "ERROR: can't read %d points for surface\n";            goto fail;
        }
        off += 0xc;
    }
    if (ReadShort(f, &number) != 1) {
        fprintf(stderr, "ERROR: can't read number of faces in surface\n");
        return 0;
    }
    faceCount = number;
    if (!AllocFaces(number)) {
arg = faceCount;        msg = "ERROR: can't allocate  %d faces\n";        goto fail;
    }
    off = 0;
    i = 0;
    if (faceCount) {
        do {
            Face *fc = (Face *)((char *)faces + off);
            int got;
            got = ReadShort(f, &number);
            fc->a = number;
            got += ReadShort(f, &number);
            fc->b = number;
            got += ReadShort(f, &number);
            fc->c = number;
            got += ReadShort(f, &number);
            fc->i0 = (short)number;
            got += ReadShort(f, &number);
            fc->i1 = (short)number;
            got += ReadShort(f, &number);
            fc->i2 = (short)number;
            got += ReadFloat(f, &fc->normal.x);
            got += ReadFloat(f, &fc->normal.y);
            got += ReadFloat(f, &fc->normal.z);
            got += ReadInt(f, &fc->pad);
            if (got != 10) {
arg = i;                msg = "ERROR: can't read face[%d] for surface\n";                goto fail;
            }
            off += 0x28;
            i++;
        } while (i < faceCount);
    }
    return ReadBinaryBox(f, (BoxNode **)&tree) != 0;
fail:
    fprintf(stderr, msg, arg);
    return 0;
}
