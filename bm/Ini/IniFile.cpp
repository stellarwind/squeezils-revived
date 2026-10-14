// 10 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
// IniFile (SQZ.EXE 0x4426d0-0x443120), functions in address order.
#include <stdio.h>
#include <string.h>
#include "Ini/IniFile.h"

char *IniSkipBlanks(char *s)
{
    while (*s != 0 && (*s == ' ' || *s == '\t'))
        s++;
    return s;
}

char *IniBackOverBlanks(char *start, char *p)
{
    while (p != start && (*p == ' ' || *p == '\t'))
        p--;
    return p;
}

char *IniFindBlank(char *s)
{
    while (*s != 0 && *s != ' ' && *s != '\t')
        s++;
    return s;
}

void GameLog(const char *format, ...);          // 0x419290
extern const char kIniDuplicateMessage[];
extern const char kIniParsingMessage[];

void IniFileSection::Add(char *line, const char *section)
{
    char *eq = strchr(line, '=');
    char *key = IniSkipBlanks(line);
    IniBackOverBlanks(key, eq - 1)[1] = 0;
    char *value = IniSkipBlanks(eq + 1);
    IniBackOverBlanks(value, value + strlen(value) - 1)[1] = 0;

    String k(key);
    String v(value);
    if (values.Contains(k)) {
        const char *old = values.Get(k).c_str();
        GameLog(kIniDuplicateMessage, key, section, old, value);
    }
    values.Insert(k, v, -1);
    order.Insert(k, -1);
}

const char *IniFileSection::Find(const char *key)
{
    if (!values.Contains(String(key)))
        return 0;
    return values.Get(String(key)).c_str();
}

StringList *IniFileSection::Keys()
{
    return &order;
}

IniFileCache::~IniFileCache()
{
    for (int i = sections.keys.count - 1; ; i--) {
        if (i < 0)
            break;
        IniFileSection *section = sections.values.data[i];
        if (section)
            delete section;
        sections.RemoveAt(i);
    }
}

int IniFileCache::Load(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;
    GameLog(kIniParsingMessage, path);

    IniFileSection *current = 0;
    String currentName;
    char line[256];
    while (fgets(line, 0xff, f)) {
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = 0;
        char *p = IniSkipBlanks(line);
        if (!*p)
            continue;
        char *comment = strchr(line, ';');
        if (comment == p)
            continue;
        if (comment)
            *comment = 0;

        if (*p == '[') {
            char *name = IniSkipBlanks(p + 1);
            if (!*name)
                continue;
            char *end = IniFindBlank(name);
            char *close = strchr(name, ']');
            *(end < close ? end : close) = 0;

            current = new IniFileSection;
            currentName = String(name);
            if (sections.Contains(String(name)))
                delete sections.Get(String(name));
            sections.Insert(String(name), current, -1);
        } else if (strchr(line, '=') && current) {
            current->Add(line, currentName.c_str());
        }
    }
    fclose(f);
    return 1;
}

StringList *IniFileCache::Keys(const char *section)
{
    if (sections.Contains(String(section)))
        return sections.Get(String(section))->Keys();
    return 0;
}

const char *IniFileCache::Find(const char *section, const char *key)
{
    if (sections.Contains(String(section)))
        return sections.Get(String(section))->Find(key);
    return 0;
}

// ---- the cache of parsed files (global at 0x4c7738) ----

extern Table<String, IniFileCache *, Array<IniFileCache *>, 51> g_iniFiles;

IniFileCache *IniOpen(const char *path)
{
    if (g_iniFiles.Contains(String(path)))
        return g_iniFiles.Get(String(path));
    else {
        IniFileCache *file = new IniFileCache;
        file->Load(path);
        g_iniFiles.Insert(String(path), file, -1);
        return file;
    }
}

void IniCloseAll()
{
    for (int i = g_iniFiles.keys.count - 1; i >= 0; i--) {
        IniFileCache *file = g_iniFiles.values.data[i];
        if (file)
            delete file;
        g_iniFiles.RemoveAt(i);
    }
}
