// A collision mesh (SQZ.EXE 0x47ad20-0x47b180): vertex array, 0x28-byte face array and a spatial tree.
struct MV3 { float x, y, z; MV3() {} MV3(float a, float b, float c) : x(a), y(b), z(c) {} ~MV3(); MV3 &operator=(const MV3 &o) { x = o.x; y = o.y; z = o.z; return *this; } };   // dtor 0x401660
struct MVert { float x, y, z; MVert(); ~MVert(); };                                                      // ctor 0x457040, dtor 0x401660
struct TreeNode { void Clear(); };                                                                       // 0x47ae10
struct MFace {                                                                                           // 0x28 bytes
    int a, b, c;            // 0x00
    MV3 normal;             // 0x0c
    int i0, i1, i2;         // 0x18
    int pad;                // 0x24
    MFace();                // 0x47ad20
    ~MFace();               // 0x4053e0
};
struct Mesh {
    int vertCount;          // 0x00
    MVert *verts;           // 0x04
    int faceCount;          // 0x08
    MFace *faces;           // 0x0c
    TreeNode *tree;         // 0x10
    char pad[0x201c - 0x14];
    int p201c;
    float radius;           // 0x2020
    int p2024, p2028, p202c;
    Mesh();                 // 0x47aec0
    void FreeVerts();       // 0x47b0b0
    void FreeFaces();       // 0x47b0f0
    void FreeArrays();      // 0x47b130
    void FreeTree();        // 0x47b150
    void FreeAll();         // 0x47af00
    float Radius();         // 0x47af20
    int AllocVerts(int n);  // 0x47af30
    int AllocFaces(int n);  // 0x47aff0
};

MFace::MFace()
{
    c = 0; b = 0; a = 0;
    normal = MV3(0.0f, 0.0f, 1.0f);
    i2 = -1; i1 = -1; i0 = -1;
    pad = 0;
}

Mesh::Mesh()
{
    vertCount = 0; verts = 0; faceCount = 0; faces = 0; tree = 0;
    p201c = 0; p2024 = 0; p2028 = 0; p202c = 0; radius = 0;
}

void Mesh::FreeAll()
{
    FreeArrays();
    FreeTree();
}

float Mesh::Radius() { return radius; }

int Mesh::AllocVerts(int n)
{
    FreeVerts();
    if (n) {
        verts = new MVert[n];
        if (!verts)
            return 0;
    }
    vertCount = n;
    return 1;
}

int Mesh::AllocFaces(int n)
{
    FreeFaces();
    if (n) {
        faces = new MFace[n];
        if (!faces)
            return 0;
    }
    faceCount = n;
    return 1;
}

void Mesh::FreeVerts()
{
    if (verts)
        delete [] verts;
    vertCount = 0;
    verts = 0;
}

void Mesh::FreeFaces()
{
    if (faces)
        delete [] faces;
    faceCount = 0;
    faces = 0;
}

void Mesh::FreeArrays()
{
    FreeVerts();
    FreeFaces();
}

void Mesh::FreeTree()
{
    TreeNode *t = tree;
    if (t) {
        t->Clear();
        delete t;
        tree = 0;
    }
}
