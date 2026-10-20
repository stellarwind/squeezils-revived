// QvLib input stream state (partial): +0 FILE*, +4 line number, +0x1c file name, +0x44 push-back counter.
#include <stdio.h>
struct QvStr { char b[12]; void Assign(const char *s); };       // 0x482840
extern char kLineFmt[];                                          // 0x4c4564
class QvInput2 {
public:
    FILE *fp;                      // +0
    int line;                      // +4
    int pad8, padc, pad10, pad14, pad18;
    QvStr fileName;                // +0x1c
    char padding[0x44 - 0x28];
    int backCount;                 // +0x44
    void LineString(QvStr *dst);   // 0x4815e0
    void PutBack(char c);          // 0x481620
    void SetFileName(const char *s);   // 0x481650
};
void QvInput2::LineString(QvStr *dst)
{
    char buf[128];
    sprintf(buf, kLineFmt, line);
    dst->Assign(buf);
}
void QvInput2::PutBack(char c)
{
    if (c != -1) {
        if (backCount >= 0)
            backCount--;
        else
            ungetc(c, fp);
    }
}
void QvInput2::SetFileName(const char *s)
{
    fileName.Assign(s);
    backCount = 0;
}
