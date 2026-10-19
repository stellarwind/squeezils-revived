// 0x42cdb0: splits a ':'-separated string and hands each piece to a sink.
#include "String/String.h"
struct Sink { virtual int v0(); virtual void Add(const String &s, int flag); };
void SplitColon(const char *s, Sink *out)
{
    String str(s);
    char *p = (char *)str.c_str();
    char *start = (char *)str.c_str();
    while (*p) {
        if (*p == ':') {
            *p = 0;
            out->Add(String(start), -1);
            p++;
            start = p;
        } else
            p++;
    }
    out->Add(String(start), -1);
}
