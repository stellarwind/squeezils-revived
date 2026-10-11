// QvLib input header (0x4815d0 end-of-file test, 0x481670 file header check "#VRML V1.0 ascii").
#include <stdio.h>
class QvIn6 {
public:
    FILE *fp;                                   // +0
    int line;                                   // +4
    float version;                              // +8
    int headerDone;                             // +0xc
    int ok;                                     // +0x10
    int ReadChar(char *c);                      // 0x4810f0
    void PutBack(char c);                       // 0x481620
    int AtEnd();                                // 0x4815d0
    int ReadHeader();                           // 0x481670
};
float QvParseVersion(char *header);             // 0x480f60
void QvError(QvIn6 *in, const char *msg);       // 0x482bd0
extern char kBadHeader[];                       // 0x4c457c

int QvIn6::AtEnd() { return fp->_flag & 0x10; }

int QvIn6::ReadHeader()
{
    char buf[256];
    char c;
    headerDone = 1;
    if (ReadChar(&c)) {
        if (c == '#') {
            buf[0] = c;
            int n = 1;
            while (ReadChar(&c) && c != '\n')
                buf[n++] = c;
            buf[n] = 0;
            if (c == '\n')
                line++;
            version = QvParseVersion(buf);
            if (version != 0)
                return 1;
        } else
            PutBack(c);
    }
    QvError(this, kBadHeader);
    ok = 0;
    return 0;
}
