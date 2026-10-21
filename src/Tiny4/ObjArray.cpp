// Additional declarations preserve VC4 register allocation after shared declarations.
void PadDecl001();
void PadDecl002();
void PadDecl003();
void PadDecl004();
void PadDecl005();
void PadDecl006();
void PadDecl007();
void PadDecl008();
void PadDecl009();
void PadDecl010();
void PadDecl011();
// 6 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
// Pointer-array serialization: BufferObject this is the subobject at +0x10.
// The original stream stores raw BufferObject pointers with no pointee adjustment.
#define BUFFEROBJECT_SIGNATURES
#include "Container2/ObjectArrayViews.h"
#include "Container2/ObjectArrayStream.h"

void ObjArrayC::Write(void *stream)
{
    ByteStream *s = (ByteStream *)stream;
    s->WriteInt(&count);
    for (int i = 0; i < count; i++)
        s->WriteObject((BufferObject *)data[i]);
}

void ObjArrayC::Read(void *stream)
{
    ByteStream *s = (ByteStream *)stream;
    s->ReadInt(&count);
    if (capacity < count)
        Grow(count - capacity);
    for (int i = 0; i < count; i++)
        s->ReadObject((BufferObject **)&data[i]);
}
