// SQZ.EXE 0x46a3d0: same shape for 24-byte elements (vtable 0x4a0a2c, element dtor 0x41d790).
struct Part24 { int v[6]; Part24() {} ~Part24(); };
struct PArr24 {
    int size, count;
    Part24 *data;               // +0xc
    virtual ~PArr24();
};
PArr24::~PArr24() { delete [] data; }
