// IniFile: see IniFile.h.
#include <stdio.h>
#include <string.h>
#include "IniFile.h"

char *IniSkipBlanks(char *s)
{
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

char *IniBackOverBlanks(char *start, char *p)
{
    while (p != start && (*p == ' ' || *p == '\t')) p--;
    return p;
}

char *IniFindBlank(char *s)
{
    while (*s && *s != ' ' && *s != '\t') s++;
    return s;
}

// ---- IniFileSection ----

void IniFileSection::Add(char *line, const char *section)
{
    char *eq = strchr(line, '=');
    char *key = IniSkipBlanks(line);
    IniBackOverBlanks(key, eq - 1)[1] = 0;
    char *value = IniSkipBlanks(eq + 1);
    IniBackOverBlanks(value, value + strlen(value) - 1)[1] = 0;

    String k(key), v(value);
    if (values.Contains(k))
        GameLog(kIniDuplicateMessage, key, section, values.Get(k).c_str(), value);
    values.Insert(k, v);
    order.Insert(k);
}

const char *IniFileSection::Find(const char *key)
{
    if (!values.Contains(String(key)))
        return NULL;
    return values.Get(String(key)).c_str();      // the table still holds the text after the copy is gone
}

StringList *IniFileSection::Keys()
{
    return &order;
}

// ---- IniFileCache ----

IniFileCache::~IniFileCache()
{
    for (int i = sections.keys.count - 1; i >= 0; i--) {
        delete sections.values.data[i];
        sections.RemoveAt(i);
    }
}

int IniFileCache::Load(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    GameLog(kIniParsingMessage, path);

    IniFileSection *current = NULL;      // lines before the first [section] are ignored
    String currentName;
    char line[256];
    while (fgets(line, 0xff, f)) {
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = 0;
        char *p = IniSkipBlanks(line);
        if (!*p) continue;
        char *comment = strchr(line, ';');
        if (comment == p) continue;
        if (comment) *comment = 0;

        if (*p == '[') {
            char *name = IniSkipBlanks(p + 1);
            if (!*name) continue;
            char *end = IniFindBlank(name);
            char *close = strchr(name, ']');
            if (end >= close) end = close;          // a header without ']' writes through NULL here, as in the original
            *end = 0;

            current = new IniFileSection;
            currentName = String(name);
            if (sections.Contains(String(name)))   // a repeated section replaces the earlier one
                delete sections.Get(String(name));
            sections.Insert(String(name), current);
        } else if (strchr(line, '=') && current) {
            current->Add(line, currentName.c_str());
        }
    }
    fclose(f);
    return 1;
}

StringList *IniFileCache::Keys(const char *section)
{
    if (!sections.Contains(String(section)))
        return NULL;
    return sections.Get(String(section))->Keys();
}

const char *IniFileCache::Find(const char *section, const char *key)
{
    if (!sections.Contains(String(section)))
        return NULL;
    return sections.Get(String(section))->Find(key);
}

// ---- the cache of parsed files ----

IniFileCache *IniOpen(const char *path)
{
    IniFileTable *files = GetIniFiles();
    if (files->Contains(String(path)))
        return files->Get(String(path));
    IniFileCache *file = new IniFileCache;
    file->Load(path);
    files->Insert(String(path), file);
    return file;
}

void IniCloseAll()
{
    IniFileTable *files = GetIniFiles();
    for (int i = files->keys.count - 1; i >= 0; i--) {
        delete files->values.data[i];
        files->RemoveAt(i);
    }
}
