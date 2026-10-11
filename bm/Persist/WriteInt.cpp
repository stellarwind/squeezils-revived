// ByteStream::WriteInt / WriteFloat (0x448ac0 / 0x448b10): both copy a dword.
#include "Persist/Stream.h"

ByteStream &ByteStream::WriteInt(int *p)
{
    Reserve(4);
    *(int *)(data + writePos) = *p;
    writePos += 4;
    return *this;
}

ByteStream &ByteStream::WriteFloat(float *p)
{
    Reserve(4);
    *(int *)(data + writePos) = *(int *)p;
    writePos += 4;
    return *this;
}
