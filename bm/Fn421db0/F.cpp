// SQZ.EXE 0x421db0 (cdecl): split a text on spaces into a new list of Strings (runs of spaces are skipped).
#include <string.h>
#include "Ini/IniFile.h"

StringList *SplitWords(const char *text)
{
    StringList *list = new StringList;
    String copy(text);
    char *p = (char *)copy.c_str();
    while (*p) {
        char *space = strchr(p, 0x20);
        if (!space) {
            list->Insert(String(p), -1);
            p += strlen(p);
        } else {
            *space = 0;
            list->Insert(String(p), -1);
            p = space + 1;
            while (*p == 0x20)
                p++;
        }
    }
    return list;
}
