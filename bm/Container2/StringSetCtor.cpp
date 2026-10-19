// 0x45f610: the string-set constructor, with its base constructors inlined.
// 16 spare declarations reproduce VC4.1's register allocation (padscan.sh).
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
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#define SET_DEFAULT_CTOR_OUTOFLINE
#include "Container/ContainerSet.h"

template <class T, int ID>
Set<T, ID>::Set() {}

template class Set<String, 11>;
