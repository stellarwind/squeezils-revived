// SQZ.EXE 0x44ad60: runs the pre-pass, then asks every object of the two global lists (virtual slot 18) until one reports a negative result.
struct Thing {
    virtual long __stdcall m0(); virtual long __stdcall m1(); virtual long __stdcall m2(); virtual long __stdcall m3(); virtual long __stdcall m4(); virtual long __stdcall m5();
    virtual long __stdcall m6(); virtual long __stdcall m7(); virtual long __stdcall m8(); virtual long __stdcall m9(); virtual long __stdcall m10(); virtual long __stdcall m11();
    virtual long __stdcall m12(); virtual long __stdcall m13(); virtual long __stdcall m14(); virtual long __stdcall m15(); virtual long __stdcall m16(); virtual long __stdcall m17();
    virtual long __stdcall Check();
};
extern int g_enabled;           // 0x4bd95c
extern int g_countA;            // 0x4c77e4
extern Thing **g_listA;         // 0x4c77ec
extern int g_countB;            // 0x4c77d4
extern Thing **g_listB;         // 0x4c77dc
void Prepare();                 // 0x45aba0
void F44ad60()
{
    int i;
    Prepare();
    if (!g_enabled)
        return;
    for (i = 0; i < g_countA; i++)
        if (g_listA[i]->Check() < 0)
            return;
    for (i = 0; i < g_countB; i++)
        if (g_listB[i]->Check() < 0)
            return;
}
