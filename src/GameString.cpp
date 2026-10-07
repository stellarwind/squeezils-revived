// GameString: see GameString.h.
#include <string.h>
#include <stdlib.h>
#include <new>
#include "GameString.h"
#include "Original.h"

StringRep::StringRep()
{
    buf = NULL;
    cap = 0;
    Reserve(0x20);
    strcpy(buf, kEmptyString);
    refs = 0;
}

bool StringRep::Reserve(unsigned n)
{
    if (cap >= n) return true;
    char *p = (char *)Operator_New(n);
    if (!p) return false;
    p[0] = 0;
    if (buf) {
        strcpy(p, buf);
        Operator_Delete(buf);
    }
    buf = p;
    cap = n;
    return true;
}

String::String(const char *s) : rep(NULL)
{
    Assign(s, (int)strlen(s));
}

void String::Release()
{
    if (!rep) return;
    rep->refs--;
    if (rep->refs != 0) return;          // the handle keeps its pointer when others still share the rep
    Operator_Delete(rep->buf);
    Operator_Delete(rep);
    rep = NULL;
}

void String::MakeUnique()
{
    StringRep *old = rep;
    if (!old) {
        void *m = Operator_New(sizeof(StringRep));
        rep = m ? new (m) StringRep() : NULL;
        rep->refs++;
        return;
    }
    if (old->refs <= 1) return;
    void *m = Operator_New(sizeof(StringRep));
    rep = m ? new (m) StringRep() : NULL;
    rep->Reserve(old->cap);
    strcpy(rep->buf, old->buf);
    rep->refs = 1;
    old->refs--;
}

char *String::Assign(const char *s, int n)
{
    MakeUnique();
    unsigned need = (unsigned)n + 1;
    if (rep->Reserve(need))
        memcpy(rep->buf, s, (size_t)n);
    rep->buf[n] = 0;
    return rep->buf;
}

String &String::operator=(const String &o)
{
    if (rep == o.rep) return *this;
    Release();
    rep = o.rep;
    if (rep) rep->refs++;
    return *this;
}

const char *String::c_str() const
{
    return rep ? rep->buf : kEmptyString;
}

bool String::ToFloat(float *out) const
{
    double v = atof(rep->buf);
    *out = (float)v;
    if (v == 0.0 || v != v)               // the original tests the x87 "equal or unordered" flag
        return strchr(rep->buf, '0') != NULL;
    return true;
}

bool StringLess(const String *a, const String *b)    { return _stricmp(a->c_str(), b->c_str()) < 0; }
bool StringEqual(const String *a, const String *b)   { return _stricmp(a->c_str(), b->c_str()) == 0; }
int  StringCompare(const String *a, const String *b)
{
    int d = _stricmp(a->c_str(), b->c_str());
    return d < 0 ? -1 : d > 0 ? 1 : 0;
}
