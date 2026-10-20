// spare declarations: the compiler's register choices follow the symbol count (tools/flagsearch/padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
// Array<String> copy constructor (SQZ.EXE 0x45e0e0).
#define CONTAINER_DEFINITIONS
#define ARRAY_COPY_CTOR
#include "Container/ContainerArray.h"
#include "String/String.h"

template class Array<String>;
