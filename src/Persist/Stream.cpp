// ByteStream (0x448a70-0x448ca0), contiguous in the original.
#include <stdlib.h>
#include <string.h>
#include "Persist/Stream.h"

ByteStream &ByteStream::ReadInt(int *p)
{
    *p = *(int *)(data + readPos);
    readPos += 4;
    return *this;
}

ByteStream &ByteStream::ReadFloat(float *p)
{
    *(int *)p = *(int *)(data + readPos);
    readPos += 4;
    return *this;
}

ByteStream &ByteStream::ReadString(char **s)
{
    int len = strlen(data + readPos) + 1;
    *s = data + readPos;
    readPos += len;
    return *this;
}

ByteStream &ByteStream::WriteObject(BufferObject *o)
{
    char *name = (char *)o->TypeName();
    WriteString(&name);
    o->Write(this);
    return *this;
}

ByteStream &ByteStream::ReadObject(BufferObject **o)
{
    char *name;
    ReadString(&name);
    BufferObject *proto = FindBufferObject(name);
    *o = proto->Create();
    (*o)->Read(this);
    return *this;
}
