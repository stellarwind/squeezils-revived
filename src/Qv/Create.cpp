// QvLib node creation by class name (0x47f090) with the string compare helpers (0x482a10 not-equal, 0x4829c0 equal).
#include <string.h>
struct QvEntryC { char *str; };
struct QvNameC { QvEntryC *e; };
struct QvNodeC { int dummy; };
struct QvStrC {
    char *data; int cap; char buf[32]; // SIZE 0x28; data +0, cap +4, buf +8
    QvStrC() { data = buf; }
    ~QvStrC();                                           // 0x482730
};
struct QvGenericC : QvNodeC {
    char b[0x1c];
    QvGenericC();                                        // 0x482c70
    void SetClass(char *name);                           // 0x482df0
};
class QvInputC {
public:
    int ReadString(QvStrC *s);                           // 0x4811d0
    void SetFileName(char *s);                           // 0x481650
};
QvNodeC *QvFactory(QvNameC *name);                       // 0x47f1d0 (cdecl)
void QvError(QvInputC *in, const char *fmt, ...);        // 0x482bd0
int QvStrNe(QvStrC *s, char *b);                         // 0x482a10
int QvStrEq(QvStrC *s, char *b);                         // 0x4829c0
extern char kExtension[];                                // 0x4c42a4
extern char kUnknownClass[];                             // 0x4c436c

int QvStrNe(QvStrC *s, char *b)
{
    char *d = s->data;
    if (*b != *d || strcmp(d, b) != 0)
        return 1;
    return 0;
}
QvNodeC *QvCreateNode(QvInputC *in, QvNameC *name)
{
    QvStrC s;
    s.buf[0] = 0;
    QvNodeC *result = QvFactory(name);
    if (!result) {
        if (!in->ReadString(&s) || QvStrNe(&s, kExtension)) {
            QvError(in, kUnknownClass, name->e->str);
            return 0;
        }
        if (QvStrEq(&s, kExtension)) {
            QvGenericC *g = new QvGenericC;
            g->SetClass(name->e->str);
            result = g;
            in->SetFileName(s.data);
        }
    }
    return result;
}
