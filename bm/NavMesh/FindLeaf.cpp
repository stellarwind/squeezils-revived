// SQZ.EXE 0x47d560: recursive octree lookup of the leaf box containing a point.
// Preserve the original infinite wait if a null root is requested.

struct NavBox {
    float bounds[6];                        // xMin/xMax/yMin/yMax/zMin/zMax
    int pad18[2];
    NavBox *children[8];                    // +0x20
};

int Outcode(float *point, float xMin, float xMax, float yMin, float yMax,
            float zMin, float zMax);        // 0x47d4f0

struct NavMeshTree {
    char pad00[0x10];
    NavBox *root;
    NavBox *FindLeaf(float *point, NavBox *box);
};

NavBox *NavMeshTree::FindLeaf(float *point, NavBox *box)
{
    if (!box) {
        box = root;
        while (!box) {}
    }
    if (Outcode(point, box->bounds[0], box->bounds[1], box->bounds[2],
                box->bounds[3], box->bounds[4], box->bounds[5]))
        return 0;
    int hasChildren = 0;
    for (int i = 0; i < 8; ++i) {
        if (box->children[i]) {
            hasChildren = 1;
            NavBox *leaf = FindLeaf(point, box->children[i]);
            if (leaf)
                return leaf;
        }
    }
    return hasChildren ? 0 : box;
}
