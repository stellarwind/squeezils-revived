// QvLib parser: node body "{ ... }" with the optional "<keyword> <name> <class>" prefix (0x47ee40).
#include <string.h>
struct QvEntryR { char *str; };
struct QvNameR { QvEntryR *e; QvNameR(); ~QvNameR(); };          // 0x482560 / 0x47ec60
struct QvNodeR;
class QvInputR {
public:
    int ReadName(QvNameR *n, int ident);                         // 0x481300
    int ReadNext(char *c);                                       // 0x4811a0
};
void QvError(QvInputR *in, const char *fmt, ...);                // 0x482bd0
void QvSkipBlock(QvInputR *in);                                  // 0x480490
int QvCreateAndRead(QvInputR *in, QvNameR *name, QvNameR *kw, QvNodeR **out); // 0x47f040
extern char kKeyword[];                                          // 0x4c4368
extern char kMissingNames[];                                     // 0x4c4348
extern char kEmptyKeyword[];                                     // 0x4c4330
extern char kEmptyName[];                                        // 0x4c4314
extern char kExpected[];                                         // 0x4c42fc
extern char kMissing[];                                          // 0x4c42e4
int QvReadNodeBody2(QvInputR *in, QvNameR *name, QvNodeR **out)
{
    QvNameR n2;
    int skip = 0;
    int ok = 1;
    int eq;
    char c;
    *out = 0;
    eq = (*name->e->str == kKeyword[0] && strcmp(name->e->str, kKeyword) == 0);
    if (eq) {
        if (!in->ReadName(&n2, 0) || !in->ReadName(name, 1)) {
            QvError(in, kMissingNames);
            ok = 0;
        }
        if (*n2.e->str == 0) {
            QvError(in, kEmptyKeyword, kKeyword);
            ok = 0;
        }
        if (*name->e->str == 0) {
            QvError(in, kEmptyName, n2.e->str);
            ok = 0;
        }
    }
    if (ok) {
        int r = in->ReadNext(&c);
        if (r && c == '{') {
            ok = QvCreateAndRead(in, name, &n2, out);
            if (!ok) {
                skip = 1;
            } else {
                r = in->ReadNext(&c);
                if (r && c == '}') {
                } else {
                    if (r)
                        QvError(in, kExpected, '}', (int)c);
                    else
                        QvError(in, kMissing, '}');
                    ok = 0;
                }
            }
        } else {
            if (r)
                QvError(in, kExpected, '{', (int)c);
            else
                QvError(in, kMissing, '{');
            ok = 0;
        }
    }
    if (!ok && skip)
        QvSkipBlock(in);
    return ok;
}
