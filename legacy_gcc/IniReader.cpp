// IniReader: see IniReader.h.
#include <string.h>
#include <stdlib.h>
#include <new>
#include "IniReader.h"
#include "Original.h"

IniReader::IniReader()
{
    path = (char *)Operator_New(0xff);
    section = (char *)Operator_New(0xff);
    strcpy(section, kDefaultSectionName);
    strcpy(path, kCurrentIniPath);
    file = IniOpen(path);
}

IniReader::IniReader(const char *f)
{
    path = (char *)Operator_New(0xff);
    section = (char *)Operator_New(0xff);
    strcpy(section, kDefaultSectionName);
    strcpy(path, f);
    file = IniOpen(path);
}

IniReader::~IniReader()
{
    Operator_Delete(path);
    Operator_Delete(section);
}

const char *IniReader::Lookup(const char *sect, const char *key) const
{
    return file->Find(sect, key);
}

const char *IniReader::GetString(const char *key) const
{
    const char *v = Lookup(section, key);
    return v ? v : kEmptyString;
}

int IniReader::GetInt(const char *key, int def) const
{
    const char *v = Lookup(section, key);
    return v ? atoi(v) : def;
}

int IniReader::GetFloats(const char *key, float *out) const
{
    const char *v = Lookup(section, key);
    return v ? IniParseFloats(v, out, NULL) : 0;
}

float IniReader::GetFloat(const char *key, float def) const
{
    const char *v = Lookup(section, key);
    return v ? (float)atof(v) : def;
}

IntList *IniReader::GetIntList(const char *key) const
{
    return ParseIntList(Lookup(section, key));
}

void IniReader::SetSection(const char *name)
{
    strcpy(section, name);
}

String *IniReader::GetDirectory(String *result) const
{
    char *slash = strrchr(path, '\\');
    if (!slash) {
        new (result) String(kCurrentDirectory);
        return result;
    }
    *slash = 0;
    String dir(path);
    *slash = '\\';
    new (result) String(dir);
    return result;
}

const char *IniReader::GetPath() const
{
    return path;
}

StringList *IniReader::GetKeys() const
{
    StringList *list = new StringList;
    StringList *keys = file->Keys(section);
    if (keys)
        for (int i = 0; i < keys->count; i++)
            list->Insert(keys->At(i));
    return list;
}

IntList *IniReader::ParseIntList(const char *s) const
{
    IntList *list = new IntList;
    for (;;) {
        while (*s == ' ') s++;
        if (!*s) break;
        list->Insert(atoi(s));
        while (*s && *s != ' ') s++;
    }
    return list;
}

int IniParseFloats(const char *s, float *out, const char **end)
{
    int n;
    for (n = 0; n < 3; n++) {
        while (*s == ' ') s++;
        out[n] = (float)atof(s);
        while (*s && *s != ' ') s++;
    }
    if (end) *end = s;
    return n;
}
