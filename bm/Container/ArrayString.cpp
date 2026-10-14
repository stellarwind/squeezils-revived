// 28 spare declarations (symbol count, see padscan.sh)
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
void PadDecl18();
void PadDecl19();
void PadDecl20();
void PadDecl21();
void PadDecl22();
void PadDecl23();
void PadDecl24();
void PadDecl25();
void PadDecl26();
void PadDecl27();
void PadDecl28();
// Array<String> (SQZ.EXE 0x45e0e0-0x45e490, Get at 0x40ec10), one instantiation in its own translation unit.
#define CONTAINER_DEFINITIONS
#define ARRAY_GET_VIA_LOCAL
#include "Container/ContainerArray.h"
#include "String/String.h"

int Array<String>::Grow(int extra)
{
    if (count + extra >= capacity) {
        capacity += capacity >> 1;
        String *fresh = new String[capacity];
        int i;
        i = 0;
        while (i < count) {
            fresh[i] = data[i];
            ++i;
        }
        delete [] data;
        data = fresh;
        return 1;
    }
    return 0;
}

template class Array<String>;
