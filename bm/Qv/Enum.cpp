// 6 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
// QvLib enum helpers on the field data: whitespace-trimmed name interning (0x480a50) and enum type lookup (0x480b50).
#include <string.h>
#include <ctype.h>
struct QvNameE {
    void *p;
    QvNameE(void *x) { p = x; }
    ~QvNameE();                                        // 0x47ec60
};
void *QvInternE(const char *s);                        // 0x482390
QvNameE QvTrimName(const char *s)
{
    int i = 0;
    int first = -1;
    int last = -1;
    int lastSpace = -1;
    if (s[0]) {
        do {
            if (isspace(s[i]))
                lastSpace = i;
            else {
                if (first == -1)
                    first = i;
                last = i;
            }
            i++;
        } while (s[i]);
    }
    if (lastSpace < last)
        return QvNameE(QvInternE(s + first));
    char buf[512];
    int n = 0;
    if (last >= first) {
        n = last - first + 1;
        memcpy(buf, s + first, n);
    }
    buf[n] = 0;
    return QvNameE(QvInternE(buf));
}
struct QvEnumTypeE { void *name; int count; int cap; int *values; void *names; };
struct QvFieldDataE {
    int pad[3];
    QvEnumTypeE **enums;                               // +0xc
    int enumCount;                                     // +0x10
    void getEnum(const char *type, int *a, int *b, int *c);
};
void QvFieldDataE::getEnum(const char *type, int *a, int *b, int *c)
{
    QvNameE n = QvTrimName(type);
    int i = 0;
    int count = enumCount;
    for (;;) {
        if (i >= count)
            break;
        QvEnumTypeE *t = enums[i];
        if (t->name == n.p) {
            *a = t->count;
            *b = (int)t->values;
            *c = (int)t->names;
            return;
        }
        i++;
    }
    *a = 0;
    *b = 0;
    *c = 0;
}
