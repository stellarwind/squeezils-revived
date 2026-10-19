// SQZ.EXE 0x46ed30: default constructor for the two-vector Pair.
// Used by the array constructor helpers alongside Pair::~Pair at 0x41d790.
#include "compat41.h"

struct Float3 {
    float x, y, z;
    Float3() {}
    ~Float3();                             // 0x401660
};

struct Pair {
    Float3 a;
    Float3 b;
    Pair();
    ~Pair();                               // 0x41d790
};

Pair::Pair() {}
