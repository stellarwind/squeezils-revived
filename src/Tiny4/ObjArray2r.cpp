// 6 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
// Serialisation of an array of BufferObject pointers (SQZ.EXE 0x465010 Read, 0x465070 Write): the BufferObject interface sits
// after the array base, so `this` in these methods points 0x10 into the object.
#include "Persist/Stream.h"
#include "Container/ContainerArray.h"

struct ObjArray : Array<BufferObject *>, BufferObject {
    virtual const char *TypeName();
    virtual BufferObject *Create();
    virtual void Write(ByteStream *s);
    virtual void Read(ByteStream *s);
};

void ObjArray::Write(ByteStream *s)
{
    s->WriteInt(&count);
    for (int i = 0; i < count; i++)
        s->WriteObject(data[i]);
}

void ObjArray::Read(ByteStream *s)
{
    s->ReadInt(&count);
    if (capacity < count)
        Grow(count - capacity);
    for (int i = 0; i < count; i++)
        s->ReadObject(&data[i]);
}
