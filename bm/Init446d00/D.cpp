#include "compat41.h"
struct Float3 { float x, y, z; ~Float3(); };
struct Pair { Float3 a; Float3 b; Pair() {} };
Pair g_pair;
