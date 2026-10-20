// SQZ.EXE 0x46a330 (default constructor) and 0x46a3d0 (destructor): array of 24-byte parts (vtable 0x4a0a2c, element dtor 0x41d790).
struct Part24 { int v[6]; Part24() {} ~Part24(); };
struct PArr24 {
    int count, cap;
    Part24 *data;               // +0xc
    PArr24();
    virtual ~PArr24();
};
PArr24::PArr24()
{
    count = 0;
    cap = 2;
    data = new Part24[cap];
}
PArr24::~PArr24() { delete [] data; }
