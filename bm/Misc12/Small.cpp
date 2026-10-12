// Leaf functions: view size as floats (SQZ.EXE 0x401c30), a zero test (0x4045a0), two adjustor thunks (0x404f80, 0x404f90).
#include "compat41.h"

struct Sized {
    char pad[0x2ac];
    int w, h;                                // 0x2ac 0x2b0
    int Dims(float *out);
};
int Sized::Dims(float *out)
{
    out[0] = (float)w;
    out[1] = (float)h;
    return 0x18;
}

struct Flag { int IsZero(int v); };
int Flag::IsZero(int v) { return v == 0; }

struct Prim { void RunA(); void RunB(); };   // 0x4044a0 0x4503f0
struct Sec {
    void RunA();
    void RunB();
};
void Sec::RunA() { ((Prim *)((char *)this - 4))->RunA(); }
void Sec::RunB() { ((Prim *)((char *)this - 4))->RunB(); }
