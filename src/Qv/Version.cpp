// QvLib file header version table (0x480f60) and the input stream constructor (0x480fc0).
#include <string.h>
#include <stdio.h>
struct QvVersion { const char *header; float version; };
struct QvVersionTable { QvVersion v[1]; float none; };
QvVersionTable g_versions = { { { "#VRML V1.0 ascii", 1.0f } }, 0.0f };      // 0x4a11e8
float QvParseVersion(char *header)
{
    int i = 0;
    for (QvVersion *v = g_versions.v; v < g_versions.v + 1; v++, i++)
        if (strcmp(v->header, header) == 0)
            return g_versions.v[i].version;
    return g_versions.none;
}

struct QvDict8 { int n; void *b; QvDict8(int nb); ~QvDict8(); };       // 0x482180 / 0x4821c0
struct QvStrS8 { char *data; int pad; char buf[32]; QvStrS8() { data = buf; buf[0] = 0; } ~QvStrS8(); };     // 0x482730
// SIZE 0x48: canonical QvInput prefix, including the push-back index.
class QvInput8 {
public:
    FILE *fp; int line; float version; int headerDone; int ok; // +0,+4,+8,+0x0c,+0x10
    QvDict8 dict;                              // +0x14
    QvStrS8 text;                              // +0x1c
    int backBufIndex;                         // +0x44
    QvInput8();
    void Init(FILE *f);                        // 0x4810c0
};
typedef char QvInput8SizeCheck[sizeof(QvInput8) == 0x48 ? 1 : -1];
QvInput8::QvInput8() : dict(251)
{
    Init(stdin);
}
