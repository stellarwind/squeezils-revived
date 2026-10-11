// QvLib string token reader (0x4811d0): optionally quoted, backslash-escaped quote, read in chunks of 255 characters.
#include <stdio.h>
#include <ctype.h>
struct QvStrA { char *data; int a, b; void Reset(int n); void Append(char *s); };     // 0x482810 / 0x482960
class QvIn5 {
public:
    int pad0;
    int line;                                   // +4
    int ReadChar(char *c);                      // 0x4810f0
    void PutBack(char c);                       // 0x481620
    int Next();                                 // 0x481740
    int ReadString(QvStrA *dst);                // 0x4811d0
};
int QvIn5::ReadString(QvStrA *dst)
{
    char buf[256];
    char c;
    if (!Next())
        return 0;
    dst->Reset(1);
    if (!ReadChar(&c))
        return 0;
    int quoted = c == '"';
    if (!quoted)
        PutBack(c);
    char *p;
    int left;
    do {
        p = buf;
        left = 255;
        for (;;) {
            if (!ReadChar(p))
                break;
            if (quoted) {
                if (*p == '"')
                    break;
                if (*p == 92) {
                    if (ReadChar(&c) && c == '"')
                        *p = '"';
                    else
                        PutBack(c);
                }
                if (*p == '\n')
                    line++;
            } else if (isspace(*p)) {
                PutBack(*p);
                break;
            }
            p++;
            left--;
            if (left <= 0)
                break;
        }
        *p = 0;
        dst->Append(buf);
    } while (left == 0);
    return 1;
}
