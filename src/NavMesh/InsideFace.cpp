// 0x47c2a0 (ret 8): NavMesh::InsideFace - 1 when the point lies inside the face's three edge planes (on an edge
// counts as inside), 0 otherwise. The sign test reads the float bits, so -0 counts as 0.
struct FaceVec {
    float x, y, z;
    FaceVec() {}
    FaceVec(const FaceVec &o) : x(o.x), y(o.y), z(o.z) {}
    ~FaceVec();                                           // 0x401660
    FaceVec operator-(const FaceVec &o) const;            // 0x401ae0
    FaceVec Cross(const FaceVec &o) const;                // 0x41a350
    float Dot(const FaceVec &o) const { return x * o.x + y * o.y + z * o.z; }
};
struct NavFace {
    int vertex[3];          // 0x00
    FaceVec normal;         // 0x0c
};
struct NavMesh {
    unsigned pointCount;    // 0x00
    FaceVec *points;        // 0x04
    int InsideFace(NavFace *face, const FaceVec &point);
};
static inline int BitSign(float f)
{
    int bits = *(int *)&f;
    if (bits > 0)
        return 1;
    return ((unsigned)bits <= 0x80000000u) - 1;
}

int NavMesh::InsideFace(NavFace *face, const FaceVec &point)
{
    FaceVec p0 = points[face->vertex[0]];
    FaceVec p1 = points[face->vertex[1]];
    FaceVec p2 = points[face->vertex[2]];
    FaceVec e0 = p1 - p0;
    FaceVec e1 = p2 - p1;
    FaceVec e2 = p0 - p2;
    FaceVec c0 = face->normal.Cross(e0);
    FaceVec c1 = face->normal.Cross(e1);
    FaceVec c2 = face->normal.Cross(e2);
    FaceVec d0 = point - p0;
    FaceVec d1 = point - p1;
    FaceVec d2 = point - p2;
    float dot[3];
    dot[0] = d0.Dot(c0);
    dot[1] = d1.Dot(c1);
    dot[2] = d2.Dot(c2);
    int s0 = BitSign(dot[0]);
    int s1 = BitSign(dot[1]);
    int s2 = BitSign(dot[2]);
    int outside = s0 == -1 || s1 == -1 || s2 == -1;
    if (outside)
        return 0;
    return 1;
}
