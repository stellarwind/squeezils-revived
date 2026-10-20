// QvNode::setName (0x47e790): re-register the node under a new name, replacing characters that are not valid in a name by '_'.
#include <string.h>
struct QvEntryS { char *str; };
struct QvNameBoxS { QvEntryS *entry; QvNameBoxS(QvEntryS *e) { entry = e; } };
struct QvTmpStr {
    char *data; int cap; char buf[32];
    QvTmpStr() { data = buf; buf[0] = 0; }
    ~QvTmpStr();                                     // 0x482730
    void Append(const char *s);                      // 0x482960
};
QvEntryS *QvInternS(const char *s);                  // 0x482390
int QvIsNameStartS(char c);                          // 0x482620
int QvIsNameCharS(char c);                           // 0x482670
class QvNodeS {
public:
    virtual ~QvNodeS();
    int p4, p8, pc;
    QvNameBoxS *name;                                // +0x10
    void SetName(QvEntryS **nm);
};
void QvRemoveNameS(QvNodeS *n, char *key);           // 0x47ea90
void QvAddNameS(QvNodeS *n, char *key);              // 0x47e9d0
void QvNodeS::SetName(QvEntryS **nm)
{
    char *old = name->entry->str;
    if (*old)
        QvRemoveNameS(this, old);
    if (name)
        delete name;
    char *src = (*nm)->str;
    int bad = 0;
    if (strlen((*nm)->str) > 0 && !QvIsNameStartS(src[0]))
        bad = 1;
    for (int i = 1; (int)strlen((*nm)->str) > i && !bad; i++)
        bad = !QvIsNameCharS(src[i]);
    if (bad) {
        QvTmpStr tmp;
        if (!QvIsNameStartS(src[0]))
            tmp.Append("_");
        for (int i = 0; (int)strlen((*nm)->str) > i; ) {
            char b[2];
            b[0] = src[i];
            b[1] = 0;
            i++;
            if (!QvIsNameCharS(src[i - 1]))
                tmp.Append("_");
            else
                tmp.Append(b);
        }
        name = new QvNameBoxS(QvInternS(tmp.data));
    } else
        name = new QvNameBoxS(*nm);
    char *s = name->entry->str;
    if (*s)
        QvAddNameS(this, s);
}
