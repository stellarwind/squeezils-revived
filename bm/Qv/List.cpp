// QvLib pointer list (0x482a60-0x482b7c) and the error reporting functions (0x482b80, 0x482bd0).
#include <stdio.h>
#include <stdarg.h>
struct QvPList {
    void **items; int count; int cap;
    QvPList();                              // 0x482a60
    ~QvPList();                             // 0x482a80
    int Find(void *x);                      // 0x482a90
    void Remove(int i);                     // 0x482ac0
    void Grow(int n);                       // 0x482b10
};
QvPList::QvPList() { items = 0; cap = 0; count = 0; }
QvPList::~QvPList() { if (items) delete items; }
int QvPList::Find(void *x)
{
    for (int i = 0; i < count; i++)
        if (items[i] == x)
            return i;
    return -1;
}
void QvPList::Remove(int i)
{
    for (int j = i; j < count - 1; j++)
        items[j] = items[j + 1];
    int n = count - 1;
    if (cap < n)
        Grow(n);
    count = n;
}
void QvPList::Grow(int n)
{
    if (cap == 0)
        cap = 4;
    while (cap < n)
        cap = cap * 2;
    void **fresh = new void *[cap];
    if (items) {
        for (int i = 0; i < count; i++)
            fresh[i] = items[i];
        delete items;
    }
    items = fresh;
}

extern char kWarnFmt[];                      // 0x4c46b0
void QvWarn(const char *what, const char *fmt, ...)
{
    char buf[10000];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    fprintf(stderr, kWarnFmt, what, buf);
}
