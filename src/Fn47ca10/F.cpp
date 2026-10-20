// 0x47ca10: NavMesh::ReadAscii - header, "Points n {" list, "Faces n {" list, then the box tree.
#include <stdio.h>
struct Point { float x, y, z; };
struct Face {
    int vertex[3];          // 0x00
    float normal[3];        // 0x0c
    int neighbour[3];       // 0x18
    int type;               // 0x24
};
struct NavMesh {
    unsigned pointCount;    // 0x00
    Point *points;          // 0x04
    unsigned faceCount;     // 0x08
    Face *faces;            // 0x0c
    void *tree;             // 0x10
    int AllocPoints(int n);                                 // 0x47af30
    int AllocFaces(int n);                                  // 0x47aff0
    int ReadAsciiBox(FILE *f, void **out);                  // 0x47cc60
    int ReadAscii(FILE *f);
};
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
        if (sscanf(line, "%d %d %d %d %d %d %f %f %f %x", &fc->vertex[0], &fc->vertex[1], &fc->vertex[2], &fc->neighbour[0],
                   &fc->neighbour[1], &fc->neighbour[2], &fc->normal[0], &fc->normal[1], &fc->normal[2], &fc->type) != 10) {
            fprintf(stderr, "ERROR: can't read face[%d]\n", i);
            return 0;
        }
        off += 0x28;
        i++;
    } while (i < faceCount);
    }
    fgets(line, 256, f);
    return ReadAsciiBox(f, &tree);
}
