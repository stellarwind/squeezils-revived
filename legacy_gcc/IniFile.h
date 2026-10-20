// IniFile: the parsed INI files behind IniReader. Reconstructed from SQZ.EXE 0x4426d0-0x443120.
// A file is parsed once, the first time anything opens it, into an IniFileCache (sections by name, each an IniFileSection of
// keys and values) and kept in a global table by path for the rest of the run. IniFileSection and IniFileCache are the
// original's names (RTTI); neither has a vtable.
#pragma once
#include "Container.h"

class IniFileSection {
public:
    void        Add(char *line, const char *section);   // @442730 stores a "key = value" line (the new value wins over a duplicate)
    const char *Find(const char *key);                   // @4428a0 value text or NULL; the text stays owned by the section
    StringList *Keys();                                  // @442980 the keys in file order

    IniValueTable values;                    // 0x00 key -> value, sorted by key (case-insensitive)
    StringList order;                        // 0x30 keys in the order the file lists them (repeats included)

    static void *operator new(size_t n) noexcept { return Operator_New(n); }   // original code deletes sections too
    static void operator delete(void *p) { Operator_Delete(p); }
};

class IniFileCache {
public:
    ~IniFileCache();                                             // @4429e0 deletes the sections
    int  Load(const char *path);                                 // @442aa0 the parser; 0 when the file cannot be opened
    StringList *Keys(const char *section);                       // @442e20 NULL when there is no such section
    const char *Find(const char *section, const char *key);     // @442ee0 value text or NULL

    Table<String, IniFileSection *, Array<IniFileSection *>, 50> sections;   // 0x00 by name

    static void *operator new(size_t n) noexcept { return Operator_New(n); }
    static void operator delete(void *p) { Operator_Delete(p); }
};

IniFileCache *IniOpen(const char *path);    // @442fa0 the parsed file for a path, parsed on first use (a missing file stays cached, empty)
void IniCloseAll();                         // @4430e0 deletes every parsed file (run when the game leaves a location)

// Line helpers (blank = space or tab).
char *IniSkipBlanks(char *s);                // @4426d0 first non-blank character
char *IniBackOverBlanks(char *start, char *p);   // @4426f0 steps p back over blanks, not past start
char *IniFindBlank(char *s);                 // @442710 first blank or the end
