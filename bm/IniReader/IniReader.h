// IniReader: a cursor over one parsed INI file with a current section (SQZ.EXE 0x443120-0x4436e0), VC++ 4.1 dialect.
// Behaviour notes: Repo/src/IniReader.h.
#pragma once
#include "String/String.h"
#include "Ini/IniFile.h"

class IniReader {
public:
    IniReader();                                           // 0x443120
    IniReader(const char *file);                           // 0x4431b0
    ~IniReader();                                          // 0x443240

    const char *Lookup(const char *section, const char *key) const;   // 0x443270
    const char *GetString(const char *key) const;                     // 0x443290
    int GetInt(const char *key, int def) const;                       // 0x4432b0
    int GetFloats(const char *key, float *out) const;                 // 0x4432e0
    float GetFloat(const char *key, float def) const;                 // 0x443310
    PrimitiveArray<int, 1> *GetIntList(const char *key) const;        // 0x443350
    void SetSection(const char *name);                                // 0x443370
    String GetDirectory() const;                                      // 0x4433a0
    const char *GetPath() const;                                      // 0x443460
    StringList *GetKeys() const;                                      // 0x443470
    PrimitiveArray<int, 1> *ParseIntList(const char *text) const;     // 0x4435f0

    char *path;           // 0x0 255 byte buffer
    char *section;        // 0x4 255 byte buffer
    IniFileCache *file;   // 0x8
};

int __stdcall IniParseFloats(const char *text, float *out, const char **end);   // 0x443590
