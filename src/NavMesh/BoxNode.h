// Shared navigation spatial-tree node.
#pragma once
// SIZE 0x40: interleaved min/max bounds, face indices, then eight children.
struct BoxNode {
    float minX, maxX, minY, maxY, minZ, maxZ; // 0x00..0x14
    unsigned count; // 0x18
    int *faces; // 0x1c
    BoxNode *child[8]; // 0x20
    BoxNode(); // 0x47ade0, Misc15/Octree.cpp
    ~BoxNode(); // 0x47ae10, recursively owns children and face storage
    int Allocate(int n); // 0x47ae50
    void FreeItems(); // 0x47ae90
};
typedef char NavBoxSizeCheck[(sizeof(BoxNode) == 0x40) ? 1 : -1];
