// QvLib node name registry: a global dictionary (0x4c4238) from interned name to the list of nodes that carry it.
struct QvNodeX;
struct QvPListN {
    void **items; int count; int cap;
    QvPListN();                                  // 0x482a60
    int Find(void *x);                           // 0x482a90
    void Remove(int i);                          // 0x482ac0
    void Grow(int n);                            // 0x482b10
};
struct QvDictN {
    int Enter(unsigned key, void *value);        // 0x482230
    int Find(unsigned key, void **out);          // 0x4822e0
};
extern QvDictN *g_nameDict;                      // 0x4c4238
void QvWarn(const char *what, const char *fmt, ...);   // 0x482b80
extern char kRemoveName[];                       // 0x4c423c
extern char kRemoveNameFmt[];                    // 0x4c4250

void QvAddName(QvNodeX *node, unsigned key)      // 0x47e9d0
{
    QvPListN *found;
    QvPListN *l;
    if (!g_nameDict->Find(key, (void **)&found)) {
        l = new QvPListN;
        g_nameDict->Enter(key, l);
    } else
        l = found;
    if (l->cap < l->count + 1)
        l->Grow(l->count + 1);
    l->items[l->count] = node;
    l->count++;
}
void QvRemoveName(QvNodeX *node, unsigned key)   // 0x47ea90
{
    QvPListN *l;
    int found = g_nameDict->Find(key, (void **)&l);
    if (found) {
        int i = l->Find(node);
        if (i < 0)
            found = 0;
        else
            l->Remove(i);
    }
    if (!found)
        QvWarn(kRemoveName, kRemoveNameFmt, key, node);
}
