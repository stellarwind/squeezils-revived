// NavMesh::NavMesh (0x47aec0): clears pointCount/points/faceCount/faces/tree, unknown201c,
// unknown2024[3] and sizeSquared (the hit list is left alone). Kept in its own TU.
struct NavMeshCtor {
    int   pointCount;    // 0x00
    void *points;        // 0x04
    int   faceCount;     // 0x08
    void *faces;         // 0x0c
    void *tree;          // 0x10
    char  pad[0x201c - 0x14];
    int   unknown201c;   // 0x201c
    float sizeSquared;   // 0x2020
    int   unknown2024[3]; // 0x2024

    NavMeshCtor();
};

NavMeshCtor::NavMeshCtor()
{
    pointCount = 0;
    points = 0;
    faceCount = 0;
    faces = 0;
    tree = 0;
    unknown201c = 0;
    unknown2024[0] = 0;
    unknown2024[1] = 0;
    unknown2024[2] = 0;
    sizeSquared = 0.0f;
}
