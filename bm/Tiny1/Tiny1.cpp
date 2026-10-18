// Tiny leaf functions (SQZ.EXE 0x47xxxx range): vptr-only dtor bodies, zeroing ctor, global clear, import call.
struct A472550 { virtual void f(); ~A472550(); };
A472550::~A472550() {}
struct A479b70 { virtual void f(); ~A479b70(); };
A479b70::~A479b70() {}

struct V471980 { int a, b, c; V471980(); };
V471980::V471980() { a = 0; b = 0; c = 0; }

int g_4c3c44;
void F47a780() { g_4c3c44 = 0; }

#include <windows.h>
#include <mmsystem.h>
void F447870() { joyReleaseCapture(0); }

struct R4c75dc { void M474f70(void *p); };
extern R4c75dc *g_4c75dc;
struct X459080 { int f0, f4, f8; void F(); };
void X459080::F() { g_4c75dc->M474f70(&f8); }

struct D43a730 { virtual void v(); ~D43a730(); void Cleanup(); };
D43a730::~D43a730() { Cleanup(); }

// 0x43a900: shutdown of a holder that owns four sub-objects and a registered callback.
struct S43d5e0 { ~S43d5e0(); };
struct S447870 { ~S447870(); };
struct S459070 { ~S459070(); void M4591b0(int); void M4592a0(int); void M4592f0(int); };
struct R4c75dc;
struct S405f90 { ~S405f90(); };
struct Reg477f60 { void Unregister(void (*cb)(), void *user); };
void Cb43aa50();
struct H43a900 {
    int f0;
    S43d5e0 *a;     // 0x04
    S459070 *c;     // 0x08
    S405f90 *d;     // 0x0c
    S447870 *b;     // 0x10
    Reg477f60 *reg; // 0x14
    char pad18[8];
    char f20[0x5c];     // 0x20
    int f7c;            // 0x7c
    char pad80[0x18];
    int f98;            // 0x98
    int Cleanup();
    void F43a9a0();
};
int H43a900::Cleanup()
{
    if (reg) { reg->Unregister(Cb43aa50, this); reg = 0; }
    delete a;
    delete b;
    delete c;
    delete d;
    a = 0; b = 0; c = 0; d = 0;
    return 1;
}

void H43a900::F43a9a0()
{
    g_4c75dc->M474f70(f20);
    if (f98) {
        c->M4591b0(2);
        c->M4592a0(1);
    }
    if (f98 == 1) {
        c->M4592f0(1);
        f7c = 1;
    } else {
        c->M4592f0(0);
        f7c = 1;
    }
}
