#include "compat41.h"
struct Float3 { float x, y, z; ~Float3(); };
struct Trio { Float3 a; Float3 b; float f; Float3 c; Trio() {} };
Trio g_trio;
