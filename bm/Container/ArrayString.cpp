// 17 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
// Array<String> (SQZ.EXE 0x45e0e0-0x45e490, Get at 0x40ec10), one instantiation in its own translation unit.
#define CONTAINER_DEFINITIONS
#define ARRAY_GET_VIA_LOCAL
#include "Container/ContainerArray.h"
#include "String/String.h"

template class Array<String>;
