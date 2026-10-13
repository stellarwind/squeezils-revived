// IniReader (SQZ.EXE 0x443120-0x4436e0), functions in address order.
#include <string.h>
#include <stdlib.h>
#include <new.h>
#include "IniReader/IniReader.h"

extern char kDefaultSectionName[];       // "Defaults"
extern char kCurrentIniPath[];           // the game's global current ini path
extern char kCurrentDirectory[];         // "."
extern char kEmptyString[];

IniReader::IniReader()
{
    path = new char[0xff];
    section = new char[0xff];
    strcpy(section, kDefaultSectionName);
    strcpy(path, kCurrentIniPath);
    file = IniOpen(path);
}

IniReader::IniReader(const char *f)
{
    path = new char[0xff];
    section = new char[0xff];
    strcpy(section, kDefaultSectionName);
    strcpy(path, f);
    file = IniOpen(path);
}

IniReader::~IniReader()
{
    delete path;
    delete section;
}

const char *IniReader::Lookup(const char *sect, const char *key) const
{
    return file->Find(sect, key);
}

const char *IniReader::GetString(const char *key) const
{
    const char *v = Lookup(section, key);
    if (v == 0)
        return kEmptyString;
    return v;
}

int IniReader::GetInt(const char *key, int def) const
{
    const char *v = Lookup(section, key);
    if (v)
        return atoi(v);
    return def;
}

int IniReader::GetFloats(const char *key, float *out) const
{
    const char *v = Lookup(section, key);
    if (v)
        return ParseFloats(v, out, 0);
    return 0;
}

float IniReader::GetFloat(const char *key, float def) const
{
    const char *v = Lookup(section, key);
    float r;
    if (v)
        r = (float)atof(v);
    else
        r = def;
    return r;
}

PrimitiveArray<int, 1> *IniReader::GetIntList(const char *key) const
{
    return ParseIntList(Lookup(section, key));
}

void IniReader::SetSection(const char *name)
{
    strcpy(section, name);
}

String IniReader::GetDirectory() const
{
    char *slash = strrchr(path, 92);
    if (!slash)
        return String(kCurrentDirectory);
    *slash = 0;
    String dir(path);
    *slash = 92;
    return dir;
}

const char *IniReader::GetPath() const
{
    return path;
}

StringList *IniReader::GetKeys() const
{
    StringList *list = new StringList;
    StringList *keys = file->Keys(section);
    if (keys) {
        for (int i = 0; i < keys->count; i++)
            list->Insert(keys->Get(i), -1);
    }
    return list;
}

int IniReader::ParseFloats(const char *s, float *out, const char **end) const
{
    int n;
    for (n = 0; n < 3; n++) {
        while (*s == ' ' && *s != 0)
            s++;
        out[n] = (float)atof(s);
        while (*s != ' ' && *s != 0)
            s++;
    }
    if (end)
        *end = s;
    return n;
}

PrimitiveArray<int, 1> *IniReader::ParseIntList(const char *s) const
{
    PrimitiveArray<int, 1> *list = new PrimitiveArray<int, 1>;
    do {
        while (*s == ' ' && *s != 0)
            s++;
        if (!*s)
            break;
        list->Insert(atoi(s), -1);
        while (*s != ' ' && *s != 0)
            s++;
    } while (*s);
    return list;
}
