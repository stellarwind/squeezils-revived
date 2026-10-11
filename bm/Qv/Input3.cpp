// QvLib input stream (0x481050-0x4811a0): constructor, character reader with push-back string, and the skip-blanks-then-read wrapper.
#include <stdio.h>
struct QvStrA { char *data; int a, b; void Reset(int n); ~QvStrA(); };    // 0x482810 / 0x482730
struct QvStrB { char b[8]; ~QvStrB(); };                                  // 0x4821c0
class QvIn3 {
public:
    void Init(FILE *f);            // 0x4810c0
    ~QvIn3();                      // 0x481050
    int ReadChar(char *c);         // 0x4810f0
    int ReadNext(char *c);         // 0x4811a0
    int Next();                    // 0x481740
    int ReadHeader();              // 0x481670
    int AtEnd();                   // 0x4815d0
    FILE *fp;                      // +0
    int line;                      // +4
    float version;                 // +8
    int headerDone;                // +0xc
    int flag10;                    // +0x10
    QvStrB tmp;                    // +0x14
    QvStrA text;                   // +0x1c  (push-back / in-memory text)
    char padding[0x44 - 0x28];
    int pos;                       // +0x44
};
void QvIn3::Init(FILE *f)
{
    version = 1.0f;
    headerDone = 0;
    pos = -1;
    fp = f;
    line = 1;
    flag10 = 1;
}
QvIn3::~QvIn3() {}
int QvIn3::ReadChar(char *c)
{
    int i = pos;
    if (i >= 0) {
        *c = text.data[i];
        pos++;
        if (*c != 0)
            return 1;
        text.Reset(1);
        pos = -1;
    }
    if (headerDone == 0 && !ReadHeader())
        return 0;
    if (AtEnd()) {
        *c = -1;
        return 0;
    }
    int ch = getc(fp);
    if (ch == -1) {
        *c = -1;
        return 0;
    }
    *c = ch;
    return 1;
}
int QvIn3::ReadNext(char *c)
{
    if (Next() && ReadChar(c))
        return 1;
    return 0;
}
