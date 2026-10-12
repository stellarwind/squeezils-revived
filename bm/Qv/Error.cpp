// QvLib error reporting with the input position (0x482bd0).
#include <stdio.h>
#include <stdarg.h>
struct QvStrE {
    char *data; int cap; char buf[32];
    QvStrE() { data = buf; buf[0] = 0; }
    ~QvStrE();                                      // 0x482730
};
class QvInputE {
public:
    void LineString(QvStrE *dst);                   // 0x4815e0
};
extern char kErrFmt[];                              // 0x4c46c8
void QvError(QvInputE *in, const char *fmt, ...)
{
    char buf[10000];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    QvStrE s;
    in->LineString(&s);
    fprintf(stderr, kErrFmt, buf, s.data);
}
