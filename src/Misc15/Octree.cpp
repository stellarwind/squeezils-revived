// A spatial-tree node (SQZ.EXE 0x47ade0-0x47ae90): eight children and a list of item indices.
#include "NavMesh/BoxNode.h"

BoxNode::BoxNode()
{
    maxX = 0; minX = 0; maxY = 0; minY = 0;
    maxZ = 0; minZ = 0; count = 0; faces = 0;
    for (int i = 0; i < 8; i++)
        child[i] = 0;
}

BoxNode::~BoxNode()
{
    FreeItems();
    for (int i = 0; i < 8; i++) {
        BoxNode *c = child[i];
        if (c) {
            delete c;
            child[i] = 0;
        }
    }
}

int BoxNode::Allocate(int n)
{
    FreeItems();
    if (n) {
        faces = new int[n];
        if (!faces)
            return 0;
        count = n;
    }
    return 1;
}

void BoxNode::FreeItems()
{
    if (faces)
        delete [] faces;
    count = 0;
    faces = 0;
}
