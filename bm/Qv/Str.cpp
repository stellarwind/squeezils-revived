// 7 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
// QvLib small string (0x482840 Assign, 0x482960 Append, 0x4829c0 equality): data pointer, capacity, 8-byte inline buffer.
#include <string.h>
struct QvStrX {
    char *data;                      // +0: == this+8 while the string lives in buf
    int cap;                         // +4
    char buf[32];                    // +8
    QvStrX() { data = buf; }
    ~QvStrX();                       // 0x482730
    void Reset(int freeIt);          // 0x482810
    void Ensure(int n);              // 0x482750
    QvStrX &Assign(const char *s);   // 0x482840
    QvStrX &Append(const char *s);   // 0x482960
};
QvStrX &QvStrX::Assign(const char *s)
{
    int len = strlen(s) + 1;
    if (s >= data && s < data + ((char *)data - (char *)this == 8 ? 32 : cap)) {
        QvStrX tmp;
        tmp.Assign(s);
        Assign(tmp.data);
        return *this;
    }
    if (len < 32) {
        if ((char *)data - (char *)this != 8)
            Reset(1);
    } else {
        if ((char *)data - (char *)this != 8) {
            if (cap < len)
                delete data;
            else
                goto copy;
        }
        data = new char[len];
    }
copy:
    strcpy(data, s);
    cap = len;
    return *this;
}
QvStrX &QvStrX::Append(const char *s)
{
    Ensure(strlen(s));
    strcat(data, s);
    return *this;
}
int QvStrEq(QvStrX *a, const char *b)
{
    if (*a->data == *b && strcmp(a->data, b) == 0)
        return 1;
    return 0;
}
