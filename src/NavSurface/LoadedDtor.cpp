// LoadedSurface::~LoadedSurface (SQZ.EXE 0x40bee0): delete the nav mesh, then destroy the type tables and the two names.
struct NavMeshL { ~NavMeshL(); };           // 0x47af00
struct StrL { int s; ~StrL(); };            // 0x4383d0
struct TypeNames { char d[0x14]; ~TypeNames(); };     // 0x40be40
struct TypeNumbers { char d[0x30]; ~TypeNumbers(); }; // 0x40be90

struct LoadedSurfaceD {
    NavMeshL *mesh;         // 0x00
    float sizeSquared;      // 0x04
    int faceCount;          // 0x08
    StrL directory;         // 0x0c
    StrL file;              // 0x10
    TypeNames typeNames;    // 0x14
    TypeNumbers typeNumbers;    // 0x28 .. 0x58
    ~LoadedSurfaceD();
};

LoadedSurfaceD::~LoadedSurfaceD()
{
    delete mesh;
}
