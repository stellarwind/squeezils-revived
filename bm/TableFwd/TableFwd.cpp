// Table<K,V>::Contains / IndexOf / RemoveAt / RemoveKey (byte-identical across every instantiation).
#include "TableFwd/TableFwd.h"

int TableFwd::Contains(const int &k)
{
    return keys.Contains(k);
}

int TableFwd::IndexOf(const int &k)
{
    return keys.IndexOf(k);
}

void TableFwd::RemoveAt(int i)
{
    keys.RemoveAt(i);
    values.Remove(i);
}

void TableFwd::RemoveKey(const int &k)
{
    int i = keys.IndexOf(k);
    if (i != -1)
        RemoveAt(i);
}
