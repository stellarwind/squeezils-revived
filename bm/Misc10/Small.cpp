// A batch of small leaf functions of the application layer.
#include "compat41.h"
#include <string.h>

struct Rng3 {                                // 0x420bc0 (ret 0xc, third argument unused)
    int a0, a4, a8, ac, a10;
    Rng3 *Init(int start, int len, int unused);
};
Rng3 *Rng3::Init(int start, int len, int unused)
{
    a8 = start;
    a0 = 0;
    a4 = start;
    a10 = len;
    ac = 0;
    return this;
}

struct Rec6 {                                // 0x420bf0
    int w[6];
    Rec6 *Copy(const Rec6 *o);
};
Rec6 *Rec6::Copy(const Rec6 *o)
{
    w[2] = o->w[2];
    w[4] = o->w[4];
    w[3] = o->w[3];
    w[0] = o->w[0];
    w[5] = o->w[5];
    w[1] = o->w[1];
    return this;
}

extern int g_flagA;                          // 0x4bae30
void SetFlagA(int v) { g_flagA = v; }        // 0x422bc0
extern int g_flagB;                          // 0x4c3c44
void SetFlagB(int v) { g_flagB = v; }        // 0x47a770

struct Stream { void Close(); };             // 0x473890
struct Owner { Stream *s; void Free(); };
void Owner::Free()                           // 0x430340
{
    Stream *p = s;
    if (p) {
        p->Close();
        delete p;
    }
}

extern char g_placeIniOverride[];            // 0x4c7348
void SetPlaceIni(const char *s) { strcpy(g_placeIniOverride, s); }   // 0x438820

struct Pad {                                 // 0x43bc10 and 0x43bc40
    char pad[0x40];
    float x;                                 // 0x40
    char pad2[0x54 - 0x44];
    float y;                                 // 0x54
    char pad3[0x74 - 0x58];
    int locked;                              // 0x74
    float TakeX();
    float TakeY();
};
float Pad::TakeX()
{
    float v;
    if (locked)
        v = 0.0f;
    else
        v = x;
    x = 0.0f;
    return v;
}
float Pad::TakeY()
{
    float v;
    if (locked)
        v = 0.0f;
    else
        v = y;
    y = 0.0f;
    return v;
}

struct Window { void GetClient(int *rc); };  // 0x474f70
extern Window *g_window;                     // 0x4c75dc
struct View {
    char pad[0x34];
    int rc[4];                               // 0x34
    int HalfW();
    int HalfH();
};
int View::HalfW() { g_window->GetClient(rc); return rc[2] / 2; }
int View::HalfH() { g_window->GetClient(rc); return rc[3] / 2; }

extern int g_pass;                           // 0x4c7798
void StepA(int p); void StepB(int p); void StepC(int p);   // 0x425310 0x4247e0 0x449d20
void RunPass(int p)                          // 0x448300
{
    g_pass++;
    StepA(p);
    StepB(p);
    StepC(p);
}

struct Obj { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void Destroy(int f); };
struct Slot {
    int pad;
    Obj *cur;
    void Set(Obj *o);
};
void Slot::Set(Obj *o)                       // 0x4489a0
{
    if (cur)
        cur->Destroy(1);
    cur = o;
}
