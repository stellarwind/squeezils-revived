// Complete destructor of a class with two Float3 members at +0 and +0xc (SQZ.EXE 0x4052b0 and 3 identical copies).
#include "compat41.h"

struct Float3 { float x, y, z; ~Float3(); };   // 0x401660
struct Pair {
    Float3 a;
    Float3 b;
    ~Pair();
};

Pair::~Pair() {}
