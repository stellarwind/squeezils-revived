// SQZ.EXE 0x437360: return-by-value wrapper that forwards its argument to a core routine (0x41a350).
struct Part { int v[3]; Part() {} ~Part(); };
struct Holder5 {
    Part Core(int a);           // 0x41a350
    Part Get(int a);            // 0x437360
};
Part Holder5::Get(int a)
{
    return Core(a);
}
