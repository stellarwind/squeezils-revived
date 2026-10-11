// QvLib tokenizer helpers on the input stream (0x481740-0x481bb0).
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
class QvIn4 {
public:
    int pad0;
    int line;                                   // +4
    int pad8;
    int headerDone;                             // +0xc
    int ReadChar(char *c);                      // 0x4810f0
    void PutBack(char c);                       // 0x481620
    int ReadHeader();                           // 0x481670
    int AtEnd();                                // 0x4815d0
    int Next();                                 // 0x481740
    int ReadLong(long *out);                    // 0x481820
    int ReadULong(unsigned long *out);          // 0x481890
    int ReadInteger(char *buf);                 // 0x4818d0
    int ReadDouble(double *out);                // 0x481950
    int ReadDigits(char *dst);                  // 0x481a40
    int ReadHexDigits(char *dst);               // 0x481ad0
    int TryChar(char *dst, char want);          // 0x481b60
};
void QvError(QvIn4 *in, const char *msg);       // 0x482bd0 (cdecl)
extern char kCommentEof[];                      // 0x4c45a8

int QvIn4::Next()
{
    char c;
    if (headerDone == 0 && !ReadHeader())
        return 0;
    for (;;) {
        int ok = ReadChar(&c);
        if (!ok)
            break;
        if (isspace(c)) {
            if (c == '\n')
                line++;
            continue;
        }
        if (!ok)
            break;
        if (c == '#') {
            do {
                if (!ReadChar(&c))
                    break;
            } while (c != '\n');
            if (AtEnd())
                QvError(this, kCommentEof);
            else
                line++;
            continue;
        }
        PutBack(c);
        break;
    }
    return 1;
}
int QvIn4::ReadLong(long *out)
{
    char buf[32];
    char *p = buf;
    if (TryChar(buf, '-') || TryChar(buf, '+'))
        p = buf + 1;
    if (!ReadInteger(p))
        return 0;
    *out = strtol(buf, 0, 0);
    return 1;
}
int QvIn4::ReadULong(unsigned long *out)
{
    char buf[32];
    if (!ReadInteger(buf))
        return 0;
    *out = strtoul(buf, 0, 0);
    return 1;
}
int QvIn4::ReadInteger(char *buf)
{
    int minLen = 1;
    char *end;
    if (TryChar(buf, '0')) {
        char *p = buf + 1;
        if (TryChar(p, 'x')) {
            end = buf + 2 + ReadHexDigits(buf + 2);
            minLen = 3;
        } else
            end = buf + ReadDigits(p) + 1;
    } else
        end = buf + ReadDigits(buf);
    if (end - buf < minLen)
        return 0;
    *end = 0;
    return 1;
}
int QvIn4::ReadDouble(double *out)
{
    char buf[32];
    int any = 0;
    int n = TryChar(buf, '-');
    if (!n)
        n = TryChar(buf, '+');
    char *p = buf + n;
    n = ReadDigits(p);
    if (n > 0) {
        any = 1;
        p += n;
    }
    if (TryChar(p, '.') > 0) {
        p++;
        n = ReadDigits(p);
        if (n > 0) {
            any = 1;
            p += n;
        }
    }
    if (!any)
        return 0;
    n = TryChar(p, 'e');
    if (!n)
        n = TryChar(p, 'E');
    if (n > 0) {
        p += n;
        n = TryChar(p, '-');
        if (!n)
            n = TryChar(p, '+');
        p += n;
        n = ReadDigits(p);
        if (n > 0)
            p += n;
        else
            return 0;
    }
    *p = 0;
    *out = atof(buf);
    return 1;
}
int QvIn4::ReadDigits(char *dst)
{
    char c;
    char *p = dst;
    while (ReadChar(&c)) {
        if (isdigit(c))
            *p++ = c;
        else {
            PutBack(c);
            break;
        }
    }
    return p - dst;
}
int QvIn4::ReadHexDigits(char *dst)
{
    char c;
    char *p = dst;
    while (ReadChar(&c)) {
        if (isxdigit(c))
            *p++ = c;
        else {
            PutBack(c);
            break;
        }
    }
    return p - dst;
}
int QvIn4::TryChar(char *dst, char want)
{
    char c;
    if (!ReadChar(&c))
        return 0;
    if (c == want) {
        *dst = c;
        return 1;
    }
    PutBack(c);
    return 0;
}
