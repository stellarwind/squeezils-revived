// String: the game's reference-counted, copy-on-write string handle (SQZ.EXE 0x438370-0x438770), VC++ 4.1 dialect.
// Behaviour notes live in Repo/src/GameString.h; this copy exists to be byte-matched with cl 10.10 (see META/BYTEMATCH_GUIDE.md).
#pragma once
#include "compat41.h"

extern char kEmptyString[];                 // 0x4b9488 in the original

struct StringRep {
    char    *buf;                           // 0x0 heap text
    unsigned cap;                           // 0x4 bytes allocated for buf
    int      refs;                          // 0x8 number of String handles pointing here

    StringRep();                            // 0x4385c0
    ~StringRep();                           // 0x438610
    bool Reserve(unsigned n);               // 0x438540
};

class String {
public:
    String();                                                    // 0x438370
    String(const char *s);                                       // 0x438380
    String(const String &o);                                     // 0x4383b0
    ~String();                                                   // 0x4383d0

    String &operator=(const String &o);                          // 0x438670
    const char *c_str() const;                                   // 0x438530
    char *Assign(const char *s, int n);                          // 0x438620
    bool ToFloat(float *out) const;                              // 0x4386a0

    void Release();                                              // 0x4384f0
    void MakeUnique();                                           // 0x4383e0

    StringRep *rep;
};

bool StringLess(const String *a, const String *b);              // 0x4386f0
bool StringEqual(const String *a, const String *b);             // 0x438720
int  StringCompare(const String *a, const String *b);           // 0x438750

// The containers order their keys with this; in the original it is StringLess itself (inlined at each use).
inline bool operator<(const String &a, const String &b) { return StringLess(&a, &b); }
