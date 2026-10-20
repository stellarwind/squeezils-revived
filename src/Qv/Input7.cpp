// QvLib name token reader (0x481300): a quoted/unquoted string via ReadString, or an identifier made of name characters.
#include <stdio.h>
struct QvName { void *p; QvName(); ~QvName(); QvName(void *x) { p = x; } };   // 0x482560 / 0x47ec60
void *QvIntern(const char *s);                                                // 0x482390
struct QvStrLocal {
    char *data; int pad; char buf[28];
    QvStrLocal() { data = buf; buf[0] = 0; }
    ~QvStrLocal();                                                            // 0x482730
};
class QvIn7 {
public:
    int pad0, line;
    int Next();                                                               // 0x481740
    int ReadChar(char *c);                                                    // 0x4810f0
    void PutBack(char c);                                                     // 0x481620
    int ReadString(QvStrLocal *s);                                            // 0x4811d0
    int ReadName(QvName *out, int ident);                                     // 0x481300
};
int QvIsNameStart(char c);                                                    // 0x482580
int QvIsNameChar(char c);                                                     // 0x4825d0
int QvIn7::ReadName(QvName *out, int ident)
{
    if (!Next())
        return 0;
    if (ident == 0) {
        QvStrLocal s;
        if (!ReadString(&s))
            return 0;
        *out = QvName(QvIntern(s.data));
        return 1;
    }
    char buf[256];
    char *p = buf;
    char c;
    int ok = ReadChar(&c);
    if (ok && QvIsNameStart(c)) {
        *p++ = c;
        for (;;) {
            ok = ReadChar(&c);
            if (!ok) break;
            if (!QvIsNameChar(c)) break;
            if (p - buf < 255)
                *p++ = c;
        }
    }
    *p = 0;
    if (ok)
        PutBack(c);
    *out = QvName(QvIntern(buf));
    return 1;
}
