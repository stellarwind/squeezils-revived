// GameString.h: String, the game's reference-counted, copy-on-write string handle. Reconstructed from SQZ.EXE 0x438370-0x438770.
// A String is one pointer to a shared StringRep; copies share the rep and bump its count, and the text is copied only when
// someone writes to a shared string (MakeUnique). Comparisons ignore case. Over 200 original functions use it (the INI
// reader, the menu and panel code, the model loader), so it sits under almost everything above the renderer.
#pragma once
#include <stddef.h>

struct StringRep {
    char    *buf;        // 0x0  heap text, always NUL terminated once constructed
    unsigned cap;        // 0x4  bytes allocated for buf
    int      refs;       // 0x8  number of String handles pointing here

    StringRep();                          // @4385c0 reserves 32 bytes and copies the empty string into them
    bool Reserve(unsigned n);             // @438540 grows buf to at least n bytes, keeping the old text
};
static_assert(sizeof(StringRep) == 0xc, "StringRep layout");

class String {
public:
    String() : rep(NULL) {}                              // @438370
    String(const char *s);                               // @438380
    String(const String &o) : rep(o.rep) { if (rep) rep->refs++; }   // @4383b0
    ~String() { Release(); }                             // @4383d0 (a thunk to Release, used by the cleanup funclets)

    String &operator=(const String &o);                  // @438670
    const char *c_str() const;                           // @438530 the shared empty string when unset
    char *Assign(const char *s, int n);                  // @438620 copy n characters of s in, return the text
    bool  ToFloat(float *out) const;                     // @4386a0 atof; false only for an unparsable (zero without any '0') string

    void  Release();                                     // @4384f0 drop this handle's reference, freeing the rep at zero
    void  MakeUnique();                                  // @4383e0 allocate or copy the rep so this handle owns it alone

    StringRep *rep;
};
static_assert(sizeof(String) == 4, "String layout");

// Free comparison helpers (the originals are cdecl taking handles by pointer). All are case-insensitive.
bool StringLess(const String *a, const String *b);       // @4386f0
bool StringEqual(const String *a, const String *b);      // @438720
int  StringCompare(const String *a, const String *b);    // @438750
