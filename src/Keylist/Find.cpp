// Keylist::NameOf (0x43db60): copies the name of the entry with the given code (a default string if none).
// Keylist::Find (SQZ.EXE 0x43dad0): linear search of the 20-byte entries by name, returns the entry's field at +0xc (0 if absent).
#include <string.h>
struct Keylist {
    void *vtbl;
    char *entries;   // +4
    int count;       // +8
    int Find(const char *name);
    void NameOf(int code, char *out);
};
int Keylist::Find(const char *name)
{
    for (int i = 0; i < count; i++)
        if (strcmp(name, entries + i * 20) == 0)
            return *(int *)(entries + i * 20 + 0xc);
    return 0;
}

void Keylist::NameOf(int code, char *out)
{
    int i;
    strcpy(out, "");
    int off = 0;
    for (i = 0; i < count; i++) {
        char *e = entries + off;
        if (*(int *)(e + 0xc) == code)
            strcpy(out, e);
        off += 20;
    }
}
