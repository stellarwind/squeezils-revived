#include "compat41.h"
struct Float3 { float x, y, z; ~Float3(); };
struct S44dfa0 { char pad[0x4c]; Float3 a; Float3 b; Float3 c; ~S44dfa0(); };
S44dfa0::~S44dfa0() {}
