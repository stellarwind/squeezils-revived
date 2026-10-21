// 0x47ca10: NavMesh::ReadAscii - header, "Points n {" list, "Faces n {" list, then the box tree.
#include <stdio.h>
#include "NavMesh/NavMesh.h"
typedef MVert Point;
typedef MFace Face;
int NavMesh::ReadAscii(FILE *f)
{
    char line[256];
    int count;
    unsigned i;
    int off;
    if (!fgets(line, 256, f)) {
        fprintf(stderr, "ERROR: can't read file header\n");
        return 0;
    }
    fgets(line, 256, f);
    if (sscanf(line, "Points %d {", &count) != 1) {
        fprintf(stderr, "ERROR: can't read points list\n");
        return 0;
    }
    if (!AllocPoints(count)) {
        fprintf(stderr, "ERROR: can't allocate %d points\n", count);
        return 0;
    }
    i = 0;
    if (pointCount) {
    off = 0;
    do {
        fgets(line, 256, f);
        Point *p = (Point *)((char *)points + off);
        if (sscanf(line, "%f %f %f", &p->x, &p->y, &p->z) != 3) {
            fprintf(stderr, "ERROR: can't read point[%d]\n", i);
            return 0;
        }
        off += 0xc;
        i++;
    } while (i < pointCount);
    }
    fgets(line, 256, f);
    fgets(line, 256, f);
    if (sscanf(line, "Faces %d {", &count) != 1) {
        fprintf(stderr, "ERROR: can't read face list\n");
        return 0;
    }
    if (!AllocFaces(count)) {
        fprintf(stderr, "ERROR: can't allocate %d faces\n", count);
        return 0;
    }
    off = 0;
    i = 0;
    if (faceCount) {
    do {
        fgets(line, 256, f);
        Face *fc = (Face *)((char *)faces + off);
        if (sscanf(line, "%d %d %d %d %d %d %f %f %f %x", &fc->a, &fc->b, &fc->c, &fc->i0,
                   &fc->i1, &fc->i2, &fc->normal.x, &fc->normal.y, &fc->normal.z, &fc->pad) != 10) {
            fprintf(stderr, "ERROR: can't read face[%d]\n", i);
            return 0;
        }
        off += 0x28;
        i++;
    } while (i < faceCount);
    }
    fgets(line, 256, f);
    return ReadAsciiBox(f, (BoxNode **)&tree);
}
