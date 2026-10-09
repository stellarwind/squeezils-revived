// String (SQZ.EXE 0x438370-0x438770), functions in address order. Byte-match target: see targets.txt.
#include <string.h>
#include <stdlib.h>
#include <new.h>
#include "String.h"

String::String() : rep(0) {}

String::String(const char *s) : rep(0) { Assign(s, (int)strlen(s)); }

String::String(const String &o) : rep(o.rep)
{
    if (rep)
        rep->refs++;
}

String::~String() { Release(); }

void String::MakeUnique()
{
    StringRep *old = rep;
    if (!old) {
        rep = new StringRep;
        rep->refs++;
        return;
    }
    int *refs = &old->refs;
    if (*refs <= 1) return;
    rep = new StringRep;
    rep->Reserve(old->cap);
    strcpy(rep->buf, old->buf);
    rep->refs = 1;
    (*refs)--;
}

void String::Release()
{
    if (!rep) return;
    rep->refs--;
    if (rep->refs != 0) return;
    delete rep;
    rep = 0;
}

const char *String::c_str() const
{
    if (!rep) return kEmptyString;
    return rep->buf;
}

bool StringRep::Reserve(unsigned n)
{
    if (cap >= n) return true;
    char *p = new char[n];
    if (!p) return false;
    p[0] = 0;
    if (buf) {
        strcpy(p, buf);
        delete buf;
    }
    buf = p;
    cap = n;
    return true;
}

StringRep::StringRep()
{
    buf = 0;
    cap = 0;
    Reserve(0x20);
    strcpy(buf, kEmptyString);
    refs = 0;
}

StringRep::~StringRep() { delete buf; }

char *String::Assign(const char *s, int n)
{
    MakeUnique();
    unsigned need = (unsigned)n + 1;
    if (rep->Reserve(need))
        strncpy(rep->buf, s, (size_t)n);
    rep->buf[need - 1] = 0;
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

bool String::ToFloat(float *out) const
{
    *out = (float)atof(rep->buf);
    if (*out == 0.0f) {
        if (!strchr(rep->buf, '0'))
            return false;
    }
    return true;
}

bool StringLess(const String *a, const String *b) { return _stricmp(a->c_str(), b->c_str()) < 0; }
bool StringEqual(const String *a, const String *b) { return _stricmp(a->c_str(), b->c_str()) == 0; }
int StringCompare(const String *a, const String *b) { return _stricmp(a->c_str(), b->c_str()); }
