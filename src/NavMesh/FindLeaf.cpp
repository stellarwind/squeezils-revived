// SQZ.EXE 0x47d560: recursive octree lookup of the leaf box containing a point.
// Preserve the original infinite wait if a null root is requested.

#include "NavMesh/NavMesh.h"

int Outcode(float *point, float xMin, float xMax, float yMin, float yMax,
            float zMin, float zMax); // 0x47d4f0, NavMesh/Outcode.cpp

BoxNode *NavMesh::FindLeaf(const Vector3f *point, BoxNode *box)
{
    if (!box) {
        box = (BoxNode *)tree;
        while (!box) {}
    }
    if (Outcode((float *)point, box->minX, box->maxX, box->minY,
                box->maxY, box->minZ, box->maxZ))
        return 0;
    int hasChildren = 0;
    for (int i = 0; i < 8; ++i) {
        if (box->child[i]) {
            hasChildren = 1;
            BoxNode *leaf = FindLeaf(point, box->child[i]);
            if (leaf)
                return leaf;
        }
    }
    return hasChildren ? 0 : box;
}
