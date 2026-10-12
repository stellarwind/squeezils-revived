// ByteStream::WriteInt / WriteFloat (0x448ac0 / 0x448b10): both copy a dword.
#include <string.h>
#include "Persist/Stream.h"

ByteStream &ByteStream::WriteInt(int *p)
{
    Reserve(4);
    memcpy(data + writePos, p, sizeof(int));
    writePos += 4;
    return *this;
}

ByteStream &ByteStream::WriteFloat(float *p)
{
    Reserve(4);
    memcpy(data + writePos, p, sizeof(float));
    writePos += 4;
    return *this;
}
