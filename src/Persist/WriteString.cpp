// 1 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
// ByteStream::WriteString (0x448bd0).
#include <string.h>
#include "Persist/Stream.h"

ByteStream &ByteStream::WriteString(char **s)
{
    int len = strlen(*s) + 1;
    Reserve(len);
    memcpy(data + writePos, *s, len);
    writePos += len;
    return *this;
}

