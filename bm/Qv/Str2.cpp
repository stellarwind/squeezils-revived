// QvLib small string capacity (0x482750), the QvName default constructor (0x482560) and the name registry on the input (0x481bc0 / 0x481bf0).
#include <string.h>
struct QvStrY {
    char *data; int cap; char buf[32];
    void Ensure(int n);                                // 0x482750
};
void QvStrY::Ensure(int n)
{
    int need = n + strlen(data) + 1;
    if (need >= 32 && ((char *)data - (char *)this == 8 || cap < need)) {
        char *nd = new char[need];
        strcpy(nd, data);
        if ((char *)data - (char *)this != 8)
            delete data;
        cap = need;
        data = nd;
    }
}
struct QvEntryY { char *str; };
void *QvInternY(const char *s);                        // 0x482390
extern char kUndefName[];                              // 0x4b9488
struct QvNameY { void *p; QvNameY *Init(); };
QvNameY *QvNameY::Init()
{
    p = QvInternY(kUndefName);
    return this;
}
struct QvNameZ { QvEntryY *e; };
struct QvNodeY;
struct QvDictY {
    int Enter(unsigned key, void *value);              // 0x482230
    int Find(unsigned key, void **out);                // 0x4822e0
};
struct QvNodeSetName { void SetName(QvNameZ *n); };    // 0x47e790
class QvInputY {
public:
    char pad[0x14];
    QvDictY dict;                                      // +0x14
    void Register(QvNameZ *n, QvNodeSetName *node);    // 0x481bc0
    QvNodeSetName *Lookup(QvNameZ *n);                 // 0x481bf0
};
void QvInputY::Register(QvNameZ *n, QvNodeSetName *node)
{
    dict.Enter((unsigned)n->e->str, node);
    node->SetName(n);
}
QvNodeSetName *QvInputY::Lookup(QvNameZ *n)
{
    QvNodeSetName *r;
    if (dict.Find((unsigned)n->e->str, (void **)&r))
        return r;
    return 0;
}
