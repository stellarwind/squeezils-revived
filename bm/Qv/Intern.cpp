// QvLib name interning (0x482390) with its hash (0x4827e0) and the small-string helpers (0x482730 dtor, 0x482810 Reset).
#include <string.h>
struct QvStrS {
    char *data;                       // points to buf while the string is short
    int pad;
    char buf[8];
    ~QvStrS();                        // 0x482730 (data != this+8 -> delete)
    void Reset(int freeIt);           // 0x482810
};
QvStrS::~QvStrS()
{
    if ((char *)data - (char *)this != 8)
        delete data;
}
void QvStrS::Reset(int freeIt)
{
    if ((char *)data - (char *)this != 8) {
        if (freeIt)
            delete data;
        data = (char *)this + 8;
    }
    *data = 0;
}
unsigned QvHash(const char *s)
{
    unsigned h = 0;
    unsigned shift = 0;
    for (; *s; s++) {
        h ^= *s << shift;
        shift += 5;
        if (shift > 24)
            shift -= 24;
    }
    return h;
}

struct QvEntry {
    char *str; unsigned hash; QvEntry *next;
    QvEntry(char *s, unsigned h, QvEntry *n) { str = s; hash = h; next = n; }
};
struct QvArena { char data[4000]; char *cur; int left; QvArena *prev; };
extern int g_nameBuckets;          // 0x4c8870
extern QvEntry **g_nameTable;      // 0x4c8868
extern QvArena *g_arena;           // 0x4c886c
void QvInitNames();                // 0x482340
char *QvStrdup(const char *s);     // 0x49bd70

QvEntry *QvIntern(const char *s)
{
    unsigned h = QvHash(s);
    if (g_nameBuckets == 0)
        QvInitNames();
    unsigned idx = h % g_nameBuckets;
    QvEntry *head = g_nameTable[idx];
    QvEntry *e = head;
    while (e) {
        if (e->hash == h && e->str[0] == s[0] && strcmp(e->str, s) == 0)
            break;
        e = e->next;
    }
    if (e == 0) {
        int len = strlen(s) + 1;
        char *copy;
        if (len >= 4000)
            copy = QvStrdup(s);
        else {
            if (g_arena == 0 || g_arena->left < len) {
                QvArena *a = (QvArena *)operator new(4012);
                a->cur = a->data;
                a->left = 4000;
                a->prev = g_arena;
                g_arena = a;
            }
            memcpy(g_arena->cur, s, len);
            copy = g_arena->cur;
            g_arena->cur += len;
            g_arena->left -= len;
        }
        e = new QvEntry(copy, h, head);
        g_nameTable[idx] = e;
    }
    return e;
}
