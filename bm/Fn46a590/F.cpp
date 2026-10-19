// spare declarations: the compiler's register choices follow the symbol count (tools/flagsearch/padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
// Array<E24>::Grow (SQZ.EXE 0x46a590): array of 24-byte elements with out-of-line constructor/destructor.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerArray.h"
struct E24 { float d[6]; E24(); ~E24(); E24 &operator=(const E24 &o) { d[0]=o.d[0]; d[1]=o.d[1]; d[2]=o.d[2]; d[3]=o.d[3]; d[4]=o.d[4]; d[5]=o.d[5]; return *this; } };
template class Array<E24>;
