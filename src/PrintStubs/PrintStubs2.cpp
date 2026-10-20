// Value print methods of the script-function classes: log a constant operand, or the base's own (fallback 0x420d90).
int DebugLogAll(const char *fmt, ...);   // 0x419290

extern const char *g_names1[];   // 0x49ec60
extern const char *g_names2[];   // 0x49ec10

struct V {
    int pad0;
    int has;          // 0x04
    int id;           // 0x08
    union { int i; float f; int idx; } a;   // 0x0c
    float y, z;       // 0x10, 0x14
    void Fallback();  // 0x420d90
    void PrintInt();      // 0x421130
    void PrintFloat();    // 0x421410
    void PrintVec();      // 0x4217c0
    void PrintName1();    // 0x4232a0
    void PrintName2();    // 0x423470
};

void V::Fallback()
{
    DebugLogAll("%d", id);
}

void V::PrintInt()
{
    if (has)
        DebugLogAll("%d", a.i);
    else
        Fallback();
}

void V::PrintFloat()
{
    if (has)
        DebugLogAll("%f", (double)a.f);
    else
        Fallback();
}

void V::PrintVec()
{
    if (has)
        DebugLogAll("%f %f %f", (double)a.f, (double)y, (double)z);
    else
        Fallback();
}

void V::PrintName1()
{
    DebugLogAll("%s", g_names1[a.idx]);
}

void V::PrintName2()
{
    DebugLogAll("%s", g_names2[a.idx]);
}
