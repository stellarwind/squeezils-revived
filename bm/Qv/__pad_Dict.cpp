void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
// 6 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
// QvLib dictionary (hash table with chained entries, key % nBuckets): 0x482180-0x482340.
struct QvDictEntry {
    unsigned key; void *value; QvDictEntry *next;
    QvDictEntry(unsigned k, void *v) { key = k; value = v; }
};
class QvDict {
public:
    int nBuckets;
    QvDictEntry **buckets;
    QvDict(int n);                                   // 0x482180
    ~QvDict();                                       // 0x4821c0
    void Clear();                                    // 0x4821e0
    int Enter(unsigned key, void *value);            // 0x482230
    int Find(unsigned key, void **out);              // 0x4822e0
    QvDictEntry **FindSlot(unsigned key);            // 0x482310
};
QvDict::QvDict(int n)
{
    nBuckets = n;
    buckets = new QvDictEntry *[n];
    for (int i = 0; i < nBuckets; i++)
        buckets[i] = 0;
}
QvDict::~QvDict()
{
    Clear();
    delete buckets;
}
void QvDict::Clear()
{
    for (int i = 0; i < nBuckets; i++) {
        QvDictEntry *e = buckets[i];
        while (e) {
            QvDictEntry *next = e->next;
            delete e;
            e = next;
        }
        buckets[i] = 0;
    }
}
int QvDict::Enter(unsigned key, void *value)
{
    QvDictEntry **slot = FindSlot(key);
    QvDictEntry *e = *slot;
    if (e == 0) {
        e = new QvDictEntry(key, value);
        *slot = e;
        e->next = 0;
        return 1;
    }
    e->value = value;
    return 0;
}
int QvDict::Find(unsigned key, void **out)
{
    QvDictEntry *e = *FindSlot(key);
    if (e == 0) {
        *out = 0;
        return 0;
    }
    *out = e->value;
    return 1;
}
QvDictEntry **QvDict::FindSlot(unsigned key)
{
    QvDictEntry **p = &buckets[key % nBuckets];
    while (*p && (*p)->key != key)
        p = &(*p)->next;
    return p;
}

// name table initialisation (0x482340): 1999 buckets
extern int g_nameBuckets;       // 0x4c8870
extern void **g_nameTable;      // 0x4c8868
extern int g_nameCount;         // 0x4c886c
void QvInitNames()
{
    g_nameBuckets = 1999;
    g_nameTable = new void *[g_nameBuckets];
    for (int i = 0; i < g_nameBuckets; i++)
        g_nameTable[i] = 0;
    g_nameCount = 0;
}
