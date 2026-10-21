// 0x47cc60 (ret 8): NavMesh::ReadAsciiBox - read one bounding-box tree node from an ascii .nav file, recursively:
// "NULL" for no node, else min and max lines, "Faces n" and n face indices, then eight children.
// 1 on success, 0 (node freed, error on stderr) on failure.
#include <stdio.h>
#include <string.h>
#pragma intrinsic(strcmp)
#include "NavMesh/BoxNode.h"
#include "NavMesh/NavMesh.h"

int NavMesh::ReadAsciiBox(FILE *f, BoxNode **out)
{
    char line[256];
    char word[64];
    int value;
    *out = 0;
    fgets(line, 0x100, f);
    if (sscanf(line, "%s", word) != 1) {
        fprintf(stderr, "ERROR: can't get start token of bbox\n");
        return 0;
    }
    if (!strcmp(word, "NULL")) {
        *out = 0;
        return 1;
    }
    BoxNode *box = new BoxNode;
    fgets(line, 0x100, f);
    if (sscanf(line, "%s %f %f %f", word, &box->minX, &box->minY, &box->minZ) != 4) {
        fprintf(stderr, "ERROR: can't read min values of bbox\n");
        if (box)
            delete box;
        return 0;
    }
    fgets(line, 0x100, f);
    if (sscanf(line, "%s %f %f %f", word, &box->maxX, &box->maxY, &box->maxZ) != 4) {
        fprintf(stderr, "ERROR: can't read max values of bbox\n");
        if (box)
            delete box;
        return 0;
    }
    fgets(line, 0x100, f);
    if (sscanf(line, "%s %d", word, &value) != 2) {
        fprintf(stderr, "ERROR: can't read number of faces of bbox\n");
        if (box)
            delete box;
        return 0;
    }
    box->Allocate(value);
    unsigned i;
    for (i = 0; i < box->count; i++) {
        fgets(line, 0x100, f);
        if (sscanf(line, "%d", &value) != 1) {
            fprintf(stderr, "ERROR: can't read face index of bbox\n");
            if (box)
                delete box;
            return 0;
        }
        box->faces[i] = value;
    }
    fgets(line, 0x100, f);
    BoxNode **child = box->child;
    for (i = 0; i < 8; i++) {
        fscanf(f, "%s", word);
        if (!ReadAsciiBox(f, child)) {
            if (box)
                delete box;
            return 0;
        }
        child++;
    }
    fgets(line, 0x100, f);
    *out = box;
    return 1;
}
