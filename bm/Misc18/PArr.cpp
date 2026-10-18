// SQZ.EXE 0x45e7b0 (default constructor) and 0x45e850 (destructor) of an array of 12-byte parts (vtable 0x4a002c): count +4, capacity +8, data +0xc.
struct Part { int v[3]; Part() {} ~Part(); };
struct PArr {
    int count, cap;
    Part *data;                 // +0xc (vptr at +0)
    PArr();
    virtual ~PArr();
};
PArr::PArr()
{
    count = 0;
    cap = 2;
    data = new Part[cap];
}
PArr::~PArr() { delete [] data; }
