// ByteStream::WriteVec3 / ReadVec3 (0x448b60 / 0x448ba0).
#include "Persist/Stream.h"

ByteStream &ByteStream::WriteVec3(Vec3 *v)
{
    Reserve(12);
    *(Vec3 *)(data + writePos) = *v;
    writePos += 12;
    return *this;
}

ByteStream &ByteStream::ReadVec3(Vec3 *v)
{
    *v = *(Vec3 *)(data + readPos);
    readPos += 12;
    return *this;
}
