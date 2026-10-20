// 0x47e5d0: lazily allocate the node-name dictionary (0x4c4238).
// QvLib uses -G3, including this initializer below the scoreboard's cutoff.
struct QvDictN {
    int nBuckets;
    void *buckets;
    QvDictN(int count); // 0x482180
};
extern QvDictN *g_nameDict;

void QvInitNameDict()
{
    if (!g_nameDict)
        g_nameDict = new QvDictN(251);
}
