// 0x46a4e0 (ret 4): Array<24-byte POD>::Remove, shift-down loop.
struct Elem24 {
    int a, b, c, d, e, f;
    Elem24& operator=(const Elem24& o) { a = o.a; b = o.b; c = o.c; d = o.d; e = o.e; f = o.f; return *this; }
};
struct Array24 {
    int vtbl_pad;   // +0
    int count;      // +4
    int capacity;   // +8
    Elem24 *data;   // +0xc
    void Remove(int i);
};
void Array24::Remove(int i)
{
    int j;
    for (j = i; j < count - 1; j++)
        data[j] = data[j + 1];
    count = count - 1;
}
