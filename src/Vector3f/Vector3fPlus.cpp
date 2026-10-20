// 6 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
// Vector3f::operator+ (0x403070). Isolated in the original; x87 scheduling order not matched yet.
#include "Vector3f.h"
Vector3f Vector3f::operator+(const Vector3f &o) const { return Vector3f(o.x + x, y + o.y, z + o.z); }
