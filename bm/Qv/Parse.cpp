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
inline int Eq(const char *a, const char *b) { return *a == *b && strcmp(a, b) == 0; }
int QvReadNode(QvInputP *in, QvNodeP **out)
{
    int result;
    QvNameP n;
    if (!in->ReadName(&n, 1)) {
        *out = 0;
        result = in->ok;
    } else {
        char *s = n.e->str;
        if (*s == 0 || Eq(s, kNullName)) {
            result = 1;
            *out = 0;
        } else if (Eq(s, kUseName)) {
            int r = QvReadUse(in);
            *out = (QvNodeP *)r;
            result = r != 0;
        } else
            result = QvReadNodeBody(in, &n, out);
    }
    return result;
}
