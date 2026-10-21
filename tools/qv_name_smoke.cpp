// Mixed reconstructed/canonical parser interfaces: run with current qv_alias.obj.
#include <QvString.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void *QvIntern(const char *s);
char *QvStrdup(const char *s);
struct QvEntry { const char *str; unsigned hash; QvEntry *next; };
struct QvArena { char data[4000]; char *cur; int left; QvArena *prev; };
extern int g_nameBuckets;
extern QvEntry **g_nameTable;
extern QvArena *g_arena;
// SIZE 0x28; the corrected local storage matches QvString.
struct QvStrX {
    char *data; // 0x00
    int cap;   // 0x04
    char buf[32]; // 0x08
    QvStrX() { data = buf; *data = 0; }
    ~QvStrX();
    QvStrX &Assign(const char *s);
    QvStrX &Append(const char *s);
    void Reset(int freeOld);
};
typedef char CheckStringSize[sizeof(QvStrX) == 40 ? 1 : -1];
struct Guarded { QvStrX value; unsigned guard; };

int main()
{
    QvName canonical("shared-name");
    QvEntry *entry = (QvEntry *)QvIntern("shared-name");
    if (entry->str != canonical.getString() || !g_arena || g_nameBuckets != 1999) return 1;
    if (g_nameTable[QvString::hash("shared-name") % g_nameBuckets] != entry) return 2;
    char longName[4101]; memset(longName, 'a', 4100); longName[4100] = 0;
    QvEntry *large = (QvEntry *)QvIntern(longName);
    QvName sameLong(longName);
    if (large->str != sameLong.getString() || strcmp(large->str, longName)) return 3;
    char *duplicate = QvStrdup(longName);
    if (!duplicate || strcmp(duplicate, longName)) return 4;
    free(duplicate);
    {
        Guarded local; local.guard = 0x12345678;
        char shortName[32]; memset(shortName, 's', 31); shortName[31] = 0;
        local.value.Assign(shortName);
        // The original assignment allocates at strlen+1 >= 32, even though
        // a 31-character string plus NUL physically fits the inline buffer.
        if (strcmp(local.value.data, shortName) || local.guard != 0x12345678) return 5;
        local.value.Assign(local.value.data + 1);
        if (strlen(local.value.data) != 30) return 6;
        local.value.Append(longName);
        if (strlen(local.value.data) != 4130 || local.guard != 0x12345678) return 7;
        local.value.Reset(1);
        if (local.value.data != local.value.buf || *local.value.data || local.guard != 0x12345678) return 8;
    }
    puts("Qv mixed-name/string smoke passed: identity, shared state, 31/4100-byte strings, self assignment, guards");
    return 0;
}
