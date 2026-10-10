// NavMesh::~NavMesh (0x47af00): frees the points and the faces via two out-of-line helpers.
// Kept in its own TU so the register-frozen Outcode (0x47d4f0) is not perturbed.
struct NavMesh {
    void Free1();    // 0x47b130 (out-of-line, points + tree)
    void Free2();    // 0x47b150 (out-of-line, faces)
    ~NavMesh();      // 0x47af00
};

NavMesh::~NavMesh()
{
    Free1();
    Free2();
}
