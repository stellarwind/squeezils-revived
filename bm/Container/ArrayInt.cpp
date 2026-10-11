// 9 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
// Array<int> (SQZ.EXE 0x45d8d0-0x45da80, Get at 0x40ec80), one instantiation in its own translation unit.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerArray.h"

template class Array<int>;
