// The parsed INI files behind IniReader (SQZ.EXE 0x4426d0-0x443120), VC++ 4.1 dialect. Behaviour: Repo/legacy_gcc/IniFile.h and META/CONTAINERS.md.
#pragma once
#include "Container/Container.h"
#include "String/String.h"

typedef BufferObjectTable<String, String, PrimitiveArray<String, 3>, 23> IniValueTable;
typedef PrimitiveArray<String, 3> StringList;

class IniFileSection {
public:
    void Add(char *line, const char *section);   // 0x442730
    const char *Find(const char *key);            // 0x4428a0
    StringList *Keys();                           // 0x442980

    IniValueTable values;                         // 0x00
    StringList order;                             // 0x30
};

class IniFileCache {
public:
    ~IniFileCache();                              // 0x4429e0
    int Load(const char *path);                   // 0x442aa0
    StringList *Keys(const char *section);        // 0x442e20
    const char *Find(const char *section, const char *key);   // 0x442ee0

    Table<String, IniFileSection *, Array<IniFileSection *>, 50> sections;   // 0x00
};

char *IniSkipBlanks(char *s);                     // 0x4426d0
char *IniBackOverBlanks(char *start, char *p);    // 0x4426f0
char *IniFindBlank(char *s);                      // 0x442710
IniFileCache *IniOpen(const char *path);          // 0x442fa0
void IniCloseAll();                               // 0x4430e0
