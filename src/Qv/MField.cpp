// QvLib multi-valued field read (0x489750): "[ v, v, ... ]" or a single value.
class QvInputM {
public:
    int ReadNext(char *c);                              // 0x4811a0
    void PutBack(char c);                               // 0x481620
};
void QvError(QvInputM *in, const char *fmt, ...);       // 0x482bd0
extern char kExpectedComma[];                           // 0x4c4ca8
extern char kCouldntRead[];                             // 0x4c4ce4
class QvMField {
public:
    virtual void v0();
    virtual void v1();
    virtual void SetNum(int n);                         // +8
    virtual void v3();
    virtual int ReadOne(QvInputM *in, int i);           // +0x10
    int p4, p8;
    int num;                                            // +0xc
    int Read(QvInputM *in);
};
int QvMField::Read(QvInputM *in)
{
    char c;
    int i = 0;
    if (in->ReadNext(&c) && c == '[') {
        if (!in->ReadNext(&c) || c != ']') {
            in->PutBack(c);
            for (;;) {
                if (num <= i)
                    SetNum(i + 1);
                if (!ReadOne(in, i++))
                    goto couldnt;
                if (!in->ReadNext(&c))
                    goto couldnt;
                if (c == ',') {
                    if (in->ReadNext(&c)) {
                        if (c == ']')
                            goto done;
                        in->PutBack(c);
                    }
                    continue;
                }
                if (c != ']')
                    goto badchar;
                break;
            }
        }
done:
        if (num > i)
            SetNum(i);
        return 1;
badchar:
        QvError(in, kExpectedComma, ',', ']', (int)c, i);
        return 0;
couldnt:
        QvError(in, kCouldntRead, i);
        return 0;
    }
    in->PutBack(c);
    SetNum(1);
    if (ReadOne(in, 0))
        return 1;
    return 0;
}
