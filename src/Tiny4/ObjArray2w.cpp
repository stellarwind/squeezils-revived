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
void PadDecl012();
// 14 spare declarations (symbol count, see padscan.sh)
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
// Pointer-array serialization: BufferObject this is the subobject at +0x10.
// The original stream stores raw BufferObject pointers with no pointee adjustment.
#define BUFFEROBJECT_SIGNATURES
#include "Container2/ObjectArrayViews.h"
#include "Container2/ObjectArrayStream.h"

void ObjArrayD::Write(void *stream)
{
    ByteStream *s = (ByteStream *)stream;
    s->WriteInt(&count);
    for (int i = 0; i < count; i++)
        s->WriteObject((BufferObject *)data[i]);
}
