// IniReader: a cursor over one parsed INI file with a current section. Reconstructed from SQZ.EXE 0x443120-0x443590.
// The game reads nearly every setting through it (193 callers of GetFloat, 112 of GetString, 98 of SetSection). The parsed
// file itself (a dictionary of sections, cached by file name) is still built and searched by original code; Lookup is the
// one call into it.
#pragma once
#include "GameString.h"

class IniReader {
public:
    IniReader();                                   // @443120 current ini file (the game's global path), section "Defaults"
    explicit IniReader(const char *file);          // @4431b0 same, for the given file
    ~IniReader();                                  // @443240 frees the two buffers (the cached parsed file stays)

    const char *Lookup(const char *section, const char *key) const;   // @443270 value text or NULL
    const char *GetString(const char *key) const;                     // @443290 value, or the shared empty string
    int         GetInt(const char *key, int def) const;               // @4432b0 atoi
    int         GetFloats(const char *key, float *out) const;         // @4432e0 up to 3 floats, 0 when the key is missing
    float       GetFloat(const char *key, float def) const;           // @443310 atof rounded to float
    void        SetSection(const char *name);                         // @443370 copies the name in (no length check)
    String     *GetDirectory(String *result) const;                   // @4433a0 path up to the last '\', or "." without one
    const char *GetPath() const;                                      // @443460

    char *path;        // 0x0 255 byte buffer
    char *section;     // 0x4 255 byte buffer
    void *file;        // 0x8 parsed file from the original's cache (IniOpen)
};
static_assert(sizeof(IniReader) == 12, "IniReader layout");

// Reads up to three space separated floats from text into out (missing ones read as 0) and reports where it stopped.
// @443590, always returns 3.
int IniParseFloats(const char *text, float *out, const char **end);
