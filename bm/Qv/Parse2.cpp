// QvLib parser: the USE reference reader (0x47ed90) and node creation + read (0x47f040).
struct QvEntryQ { char *str; };
struct QvNameQ { QvEntryQ *e; QvNameQ(); ~QvNameQ(); };         // 0x482560 / 0x47ec60
struct QvNodeQ;
class QvInputQ {
public:
    int ReadName(QvNameQ *n, int ident);                         // 0x481300
    QvNodeQ *Lookup(QvNameQ *n);                                 // 0x481bf0
    void Register(QvNameQ *n, QvNodeQ *node);                    // 0x481bc0
};
struct QvNodeQ { virtual void v0(); virtual void v1(); virtual void v2(); virtual int Read(QvInputQ *in); };   // slot 3 at +0xc
void QvError(QvInputQ *in, const char *fmt, ...);                // 0x482bd0
extern char kNoUseName[];                                        // 0x4c42c4
extern char kUnknownName[];                                      // 0x4c42ac
QvNodeQ *QvReadUse(QvInputQ *in)
{
    QvNameQ n;
    QvNodeQ *result;
    if (!in->ReadName(&n, 0)) {
        QvError(in, kNoUseName);
        result = 0;
    } else {
        result = in->Lookup(&n);
        if (!result)
            QvError(in, kUnknownName, n.e->str);
    }
    return result;
}
QvNodeQ *QvCreateNode(QvInputQ *in, QvNameQ *name);              // 0x47f090
int QvCreateAndRead(QvInputQ *in, QvNameQ *name, QvNameQ *kw, QvNodeQ **out)
{
    QvNodeQ *node = QvCreateNode(in, name);
    *out = node;
    if (!node)
        return 0;
    if (*kw->e->str)
        in->Register(kw, node);
    return (*out)->Read(in);
}
