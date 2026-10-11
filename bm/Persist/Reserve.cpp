// 8 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
// ByteStream::Reserve (0x448a70), own TU for the register-choice padding.
#include <stdlib.h>
#include "Persist/Stream.h"

void ByteStream::Reserve(int n)
{
    int grew = 0;
    int need = writePos + n;
    if (capacity <= need) {
        grew = 1;
        do {
            capacity += capacity / 2;
        } while (need >= capacity);
    }
    if (grew)
        data = (char *)realloc(data, capacity);
}

