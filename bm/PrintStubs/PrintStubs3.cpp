// 8 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
// Misc small script-function leaves: compound dump (0x421c40), run guard (0x423e20), cached show (0x426120), midi pause (0x45aba0).
#include <windows.h>
#include <mmsystem.h>

void DebugLog(const char *fmt, ...);   // 0x419290

struct Item {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual void Dump();     // slot 3
};
struct ItemArray {
    virtual Item *Get(int i);   // slot 0
    int count;                  // 0x04
};
struct Compound {
    int pad0;
    ItemArray items;            // 0x04
    void DumpAll();             // 0x421c40
};

void Compound::DumpAll()
{
    DebugLog("compound");
    for (int i = 0; i < items.count; i++) {
        DebugLog("  ");
        items.Get(i)->Dump();
    }
}

struct Target {
    int *Self();       // 0x420c50
    int Name();        // 0x420c60
};
struct Runner {
    int __stdcall Run(int unused, Target *t);   // 0x423e20
};

int __stdcall RunTarget(Target *t, int unused)
{
    int *self = t->Self();
    if (!self) {
        DebugLog("WARNING: attempt to run %s", t->Name());
        return 0;
    }
    self[0x94 / 4] = 1;
    return 1;
}

struct Cached {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual int Create(int arg);   // slot 3
    char pad[0x34 - 4];
    int cache;                     // 0x34
    int Get(int arg);              // 0x426120
};

int Cached::Get(int arg)
{
    if (!cache)
        cache = Create(arg);
    else
        DebugLog("WARNING: tried to show already-visible prop.");
    return cache;
}

extern HMIDISTRM g_midiStream;   // 0x4c86b8

int PauseMidi()
{
    if (g_midiStream) {
        if (midiStreamPause(g_midiStream)) {
            DebugLog("WARNING: midiStreamPause failed");
            return 0;
        }
        return 1;
    }
    return 0;
}
