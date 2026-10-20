// SQZ.EXE 0x47c6f0: read a 32-bit binary-surface value in host byte order.
#include <stdio.h>

int HostIsLittleEndian();                   // 0x47c670, kept out of line

int ReadInt32(FILE *file, unsigned int *out)
{
    unsigned int value;
    if (fread(&value, 4, 1, file) != 1)
        return 0;
    if (HostIsLittleEndian()) {
        *out = value;
    } else {
        *out = ((value & 0xff00) | (value << 16)) << 8
             | ((value & 0xff0000) | (value >> 16)) >> 8;
    }
    return 1;
}
