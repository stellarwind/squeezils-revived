// 0x4499e0: release everything the device registry holds, newest first.
struct ComObj {
    virtual long __stdcall m0(); virtual long __stdcall m1();
    virtual unsigned long __stdcall Release();          // slot 2 (+8)
    virtual long __stdcall m3(); virtual long __stdcall m4(); virtual long __stdcall m5();
    virtual long __stdcall m6(); virtual long __stdcall m7(); virtual long __stdcall m8();
    virtual long __stdcall m9(); virtual long __stdcall m10(); virtual long __stdcall m11();
    virtual long __stdcall m12(); virtual long __stdcall m13(); virtual long __stdcall m14();
    virtual long __stdcall m15(); virtual long __stdcall m16(); virtual long __stdcall m17();
    virtual long __stdcall Shutdown();                  // slot 18 (+0x48)
};
struct ComList {
    int vptr;
    int count;                  // +4
    int cap;                    // +8
    ComObj **data;              // +0xc
    ComObj *Get(int i) { return data[i]; }
    void Remove(int i);         // 0x468120
};
struct OtherList {
    int vptr;
    int count;
    int cap;
    void *data;
    void Remove(int i);         // 0x45e300
};
extern ComList g_4c77e0, g_4c77d0;
extern OtherList g_4c77b0;
void Shutdown4499e0()
{
    int i, j, k;
    for (;;) {
        i = g_4c77e0.count - 1;
        if (i < 0)
            break;
        g_4c77e0.Get(i)->Release();
        g_4c77e0.Remove(i);
    }
    for (j = g_4c77d0.count - 1; j >= 0; j--) {
        g_4c77d0.Get(j)->Shutdown();
        g_4c77d0.Get(j)->Release();
        g_4c77d0.Remove(j);
    }
    for (k = g_4c77b0.count - 1; k >= 0; k--)
        g_4c77b0.Remove(k);
}
