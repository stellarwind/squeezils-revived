// QvLib reference-counted node list (0x489a10 append, 0x489a40 truncate); the list header is {items, count, capacity}.
struct QvRefd { void Ref(); void Unref(); };       // 0x47e750 / 0x47e760
struct QvNodeList {
    QvRefd **items; int count; int cap;
    void Grow(int n);                              // 0x482b10
    void Append(QvRefd *n);
    void Truncate(int n);
};
void QvNodeList::Append(QvRefd *n)
{
    if (cap < count + 1)
        Grow(count + 1);
    items[count] = n;
    count++;
    n->Ref();
}
void QvNodeList::Truncate(int n)
{
    for (int i = n; i < count; i++)
        if (items[i])
            items[i]->Unref();
    count = n;
}
