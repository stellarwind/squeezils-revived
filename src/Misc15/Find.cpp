// A linear-search map lookup (SQZ.EXE 0x489da0): parallel key / value arrays, count at 0x10.
struct IntMap {
    int pad[4];
    int count;                       // 0x10
    int *values;                     // 0x14
    int *keys;                       // 0x18
    int Find(const int *key, int *out);
};
int IntMap::Find(const int *key, int *out)
{
    int i = 0;
    for (;;) {
        if (i >= count)
            return 0;
        if (keys[i] == *key)
            break;
        i++;
    }
    *out = values[i];
    return 1;
}
