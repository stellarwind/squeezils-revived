// SQZ.EXE 0x45e850: destructor of an array holder of 12-byte parts (vtable 0x4a002c, data pointer at +0xc).
struct Part { int v[3]; Part() {} ~Part(); };
struct PArr {
    int size, count;
    Part *data;                 // +0xc ... (vptr at +0)
    virtual ~PArr();
};
PArr::~PArr() { delete [] data; }
