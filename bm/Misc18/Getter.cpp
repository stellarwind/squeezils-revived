// SQZ.EXE 0x40f850 and 0x410660: return-by-value wrappers that forward to a core lookup with the object's fields.
struct Part { int v[3]; Part() {} ~Part(); };
struct Holder3 {
    int pad0;
    int m4;                     // +4
    char pad8[0x44 - 8];
    int m44;                    // +0x44
    Part Core(int idx, int *a, int b, int c);   // 0x40ce30
    Part Get(int idx);                          // 0x40f850
};
Part Holder3::Get(int idx)
{
    return Core(idx, &m4, m44, 0);
}

struct Holder4 {
    char pad0[0x48];
    int m48;                    // +0x48
    char pad4c[0x88 - 0x4c];
    int m88;                    // +0x88
    Part Core(int idx, int *a, int b, int c);   // 0x40c8a0
    Part Get(int idx);                          // 0x410660
};
Part Holder4::Get(int idx)
{
    return Core(idx, &m48, m88, 0);
}
