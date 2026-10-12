// Stream extraction of a String (SQZ.EXE 0x4387a0, ret 8): reads the next token as text and assigns it; returns the reader.
#include "compat41.h"
#include "String/String.h"

struct Reader {
    void NextText(char **out);       // 0x448c20
};

Reader *ReadStringFrom(Reader *r, String *dst)
{
    char *text;
    r->NextText(&text);
    *dst = String(text);
    return r;
}
