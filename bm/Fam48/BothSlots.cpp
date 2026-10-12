// A method of the BufferObject part of a table (SQZ.EXE 0x4606d0 and 7 identical copies): forwards its argument to
// virtual slot 3 of the two sub-objects that sit at this-0x18 (the key set) and this-0x4 (the value array).
#include "compat41.h"

struct Part { virtual void v0(); virtual void v1(); virtual void Slot2(int x); virtual void Slot3(int x); };
struct Buf {
    void Both(int x);
    void Both2(int x);
};

void Buf::Both(int x)
{
    ((Part *)((char *)this - 0x18))->Slot3(x);
    ((Part *)((char *)this - 0x4))->Slot3(x);
}

void Buf::Both2(int x)
{
    ((Part *)((char *)this - 0x18))->Slot2(x);
    ((Part *)((char *)this - 0x4))->Slot2(x);
}
