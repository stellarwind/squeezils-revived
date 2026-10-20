// 1 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
// The key-binding table (SQZ.EXE 0x43d920-0x43db4f): 0x33 entries of 0x44 bytes starting at this + 0x18.
#include <string.h>
extern char kEmptyString[];                              // 0x4b9488
struct KeyEntry {
    char before[0x18];
    int code;                                            // 0x18
    int extra;                                           // 0x1c
    int type;                                            // 0x20
    char name[0x20];                                     // 0x24
};
struct KeyTable {
    KeyEntry entries[0x33];
    int FindByNamePrefix(const char *name);              // 0x43d920 (ret 4)
    int FindByCode(int code);                            // 0x43d980 (ret 4)
    int FindByName(const char *name);                    // 0x43d9b0 (ret 4)
    int GetName(int index, char *out);                   // 0x43da00 (ret 8)
    int GetCode(int index);                              // 0x43daa0 (ret 4)
    int GetExtra(int index);                             // 0x43db40 (ret 4)
};

int KeyTable::FindByNamePrefix(const char *name)
{
    for (int i = 0; i < 0x33; i++) {
        if (strlen(entries[i].name) && !strncmp(entries[i].name, name, strlen(entries[i].name)))
            return i;
    }
    return -1;
}

int KeyTable::FindByCode(int code)
{
    for (int i = 0; i < 0x33; i++)
        if (entries[i].code == code)
            return i;
    return -1;
}

int KeyTable::FindByName(const char *name)
{
    for (int i = 0; i < 0x33; i++)
        if (!strcmp(entries[i].name, name))
            return i;
    return -1;
}

int KeyTable::GetName(int index, char *out)
{
    KeyEntry *e = &entries[index];
    if (e->type == 4) {
        strcpy(out, kEmptyString);
        return strlen(out);
    }
    strcpy(out, e->name);
    return strlen(out);
}

int KeyTable::GetCode(int index)
{
    KeyEntry *e = &entries[index];
    if (e->type != 2)
        return e->code;
    return 0;
}

int KeyTable::GetExtra(int index)
{
    return entries[index].extra;
}
