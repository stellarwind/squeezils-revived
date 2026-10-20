// 9-dword record assignment returning this (SQZ.EXE 0x435760, ret 4).
#include "compat41.h"

struct Rec9 {
    int w[9];
    Rec9 &operator=(const Rec9 &o);
};

Rec9 &Rec9::operator=(const Rec9 &o)
{
    w[0] = o.w[0]; w[1] = o.w[1]; w[2] = o.w[2];
    w[3] = o.w[3]; w[4] = o.w[4]; w[5] = o.w[5];
    w[6] = o.w[6]; w[7] = o.w[7]; w[8] = o.w[8];
    return *this;
}
