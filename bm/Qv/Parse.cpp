// QvLib node reader (0x47eb00): reads a node name and dispatches on NULL / USE / a node class.
#include <string.h>
struct QvEntryP { char *str; };
struct QvNameP { QvEntryP *e; QvNameP(); ~QvNameP(); };          // 0x482560 / 0x47ec60
class QvInputP {
public:
    int p0, p4, p8, pc;
    int ok;                                                      // +0x10
    int ReadName(QvNameP *n, int ident);                         // 0x481300
};
struct QvNodeP;
extern char kNullName[];                                         // 0x4c4068
extern char kUseName[];                                          // 0x4c427c
int QvReadUse(QvInputP *in);                                     // 0x47ed90
int QvReadNodeBody(QvInputP *in, QvNameP *n, QvNodeP **out);     // 0x47ee40
int QvReadNode(QvInputP *in, QvNodeP **out)
{
    QvNameP n;
    int result;
    int eq;
    if (!in->ReadName(&n, 1)) {
        *out = 0;
        result = in->ok;
    } else {
        char *s = n.e->str;
        if (*s == 0)
            goto nullcase;
        {
            eq = (*s == kNullName[0] && strcmp(s, kNullName) == 0);
            if (eq) {
nullcase:
                result = 1;
                *out = 0;
            } else {
                s = n.e->str;
                eq = (*s == kUseName[0] && strcmp(s, kUseName) == 0);
                if (eq) {
                    int r = QvReadUse(in);
                    *out = (QvNodeP *)r;
                    result = r != 0;
                } else
                    result = QvReadNodeBody(in, &n, out);
            }
        }
    }
    return result;
}
