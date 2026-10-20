// 0x47d2b0 (ret 8): NavMesh::ReadBinaryBox - read one bounding-box tree node (bounds, face indices, eight optional children,
// 0xffff = no child) from a binary .nav file, recursively; 1 on success, 0 (node freed, error on stderr) on failure.
#include <stdio.h>
int ReadShort(FILE *f, unsigned short *out);                // 0x47c690
int ReadFloat(FILE *f, float *out);                         // 0x47c770
struct BoxNode {
    float minX, maxX, minY, maxY, minZ, maxZ;               // 0x00..0x14
    unsigned int count;                                     // 0x18
    int *faces;                                             // 0x1c
    BoxNode *child[8];                                      // 0x20
    BoxNode();                                              // 0x47ade0
    ~BoxNode();                                             // 0x47ae10
    int Allocate(int n);                                    // 0x47ae50
};
struct NavMesh {
    int ReadBinaryBox(FILE *f, BoxNode **out);
};

int NavMesh::ReadBinaryBox(FILE *f, BoxNode **out)
{
    *out = 0;
    BoxNode *box = new BoxNode;
    ReadFloat(f, &box->minX);
    ReadFloat(f, &box->minY);
    ReadFloat(f, &box->minZ);
    ReadFloat(f, &box->maxX);
    ReadFloat(f, &box->maxY);
    ReadFloat(f, &box->maxZ);
    unsigned short value;
    if (ReadShort(f, &value) != 1) {
        fprintf(stderr, "ERROR: can't read number of faces for bbox\n");
        delete box;
        return 0;
    }
    box->count = value;
    if (!box->Allocate(value)) {
        fprintf(stderr, "ERROR: can't alloacte %d faces for bbox\n", box->count);
        delete box;
        return 0;
    }
    unsigned int i;
    for (i = 0; i < box->count; i++) {
        if (ReadShort(f, &value) != 1) {
            fprintf(stderr, "ERROR: can't read face[%d] of bbox\n", i);
            delete box;
            return 0;
        }
        box->faces[i] = value;
    }
    for (i = 0; i < 8; i++) {
        box->child[i] = 0;
        unsigned short flag;
        if (ReadShort(f, &flag) != 1) {
            delete box;
            fprintf(stderr, "ERROR: can't read child %d\n", i);
            return 0;
        }
        if (flag != 0xffff && !ReadBinaryBox(f, &box->child[i])) {
            delete box;
            return 0;
        }
    }
    *out = box;
    return 1;
}
