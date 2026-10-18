// SQZ.EXE 0x45dc70: copy constructor of a growable int array (vtable 0x49ffec): count +4, capacity +8, data +0xc.
struct IntArr {
    virtual ~IntArr();
    int count, cap;
    int *data;
    IntArr(const IntArr &o);
    IntArr(int n, const int *src);
};
IntArr::IntArr(const IntArr &o)
{
    count = o.count;
    cap = o.cap;
    data = new int[cap];
    for (int i = 0; i < count; i++)
        data[i] = o.data[i];
}

IntArr::IntArr(int n, const int *src)
{
    count = n;
    cap = n == 0 ? 2 : n;
    data = new int[cap];
    for (int i = 0; i < count; i++)
        data[i] = *src++;
}
