// 2 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
// QvLib top-level node reader (0x481c30) and the 13-bucket node table (0x481c90-0x481d90).
struct QvNodeT { virtual ~QvNodeT(); int key; QvNodeT *next; };      // key at +4, next at +8
class QvInputT {
public:
    int AtEnd();                                                    // 0x4815d0
    int ReadChar(char *c);                                          // 0x4810f0
};
int QvReadNode(QvInputT *in, QvNodeT **out);                        // 0x47eb00
void QvError(QvInputT *in, const char *fmt, ...);                   // 0x482bd0
extern char kBadChar[];                                             // 0x4c45ec
int QvReadTop(QvInputT *in, QvNodeT **out)
{
    int ok = QvReadNode(in, out);
    if (ok && *out == 0 && !in->AtEnd()) {
        char c;
        in->ReadChar(&c);
        ok = 0;
        QvError(in, kBadChar, (int)c);
    }
    return ok;
}
struct QvNodeTable {
    int count;
    QvNodeT **buckets;
    QvNodeTable();                                                  // 0x481c90
    ~QvNodeTable();                                                 // 0x481cc0
    void Insert(int idx, QvNodeT *e);                               // 0x481cf0
    void Inc();                                                     // 0x481d20
    void Sweep();                                                   // 0x481d30
    void RemoveHead(int idx);                                       // 0x481d70
};
QvNodeTable::QvNodeTable()
{
    buckets = (QvNodeT **)operator new(52);
    for (int i = 0; i < 13; i++)
        buckets[i] = 0;
    count = 0;
}
QvNodeTable::~QvNodeTable()
{
    while (count > 0)
        Sweep();
    delete buckets;
}
void QvNodeTable::Insert(int idx, QvNodeT *e)
{
    e->key = count;
    e->next = buckets[idx];
    buckets[idx] = e;
}
void QvNodeTable::Inc() { count++; }
void QvNodeTable::Sweep()
{
    count--;
    for (int i = 0; i < 13; i++) {
        while (buckets[i] && buckets[i]->key > count)
            RemoveHead(i);
    }
}
void QvNodeTable::RemoveHead(int idx)
{
    QvNodeT **p = buckets + idx;
    QvNodeT *e = *p;
    *p = e->next;
    if (e)
        delete e;
}
