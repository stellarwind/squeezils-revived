// Stream extraction of a String (SQZ.EXE 0x4387a0, ret 8): reads the next token as text and assigns it; returns the reader.
#include "compat41.h"
#include "String/String.h"
#include "Persist/Stream.h"

ByteStream *ReadElem(ByteStream *r, String *dst)
{
    char *text;
    r->ReadString(&text);
    *dst = String(text);
    return r;
}
