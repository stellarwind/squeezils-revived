// 0x4290b0: CycleProp::FireStepMessages. Preserve the original direction predicate,
// including its unreachable upward path (the computed direction is only -1/+1).
#include "String/String.h"
// SIZE 0x04
struct Base { virtual ~Base(); /* vtable+0x00 */ };
struct Target : Base { };
// SIZE 0x04
struct Events { virtual int v0(); virtual int Has(int *k); /* vtable+0x04 */ String Get(int *k); }; // 0x4638b0, hidden result
// SIZE 0x18
struct Ref { int d[6]; /* 0x00 */ Ref(int handle, void *owner); Ref(Target *t, int handle, void *owner); }; // 0x4207a0 / 0x420900
#include "Script/CallFunction.h"
struct Owner;
Owner *GetOwner(); // 0x420690
struct Owner { Base *Find(int handle); }; // 0x414080, thiscall ret 4
struct CycleProp {
    char pad[0x9c];          // 0x00
    int handle;             // 0x9c
    char pad2[0xac - 0xa0]; // 0xa0
    Events events;          // 0xac
    void FireStepMessages(int from, int to);
};
void CycleProp::FireStepMessages(int from, int to)
{
    int number;
    Events *ev;
    if ((((to - from <= 0) - 1) & 2) - 1 > 1) {
        number = from;
        if (to > from) {
            ev = &events;
            do {
                if (ev->Has(&number)) {
                    if (handle != -1) {
                        Target *t = dynamic_cast<Target *>(GetOwner()->Find(handle));
                        Ref ref(t, handle, this);
                        CallFunction(ev->Get(&number).c_str(), &ref, 0, 0);
                    } else {
                        Ref ref(-1, this);
                        CallFunction(ev->Get(&number).c_str(), &ref, 0, 0);
                    }
                }
                number++;
            } while (number < to);
        }
    } else {
        number = from;
        if (to < from) {
            ev = &events;
            do {
                if (ev->Has(&number)) {
                    if (handle != -1) {
                        Target *t = dynamic_cast<Target *>(GetOwner()->Find(handle));
                        Ref ref(t, handle, this);
                        CallFunction(ev->Get(&number).c_str(), &ref, 0, 0);
                    } else {
                        Ref ref(-1, this);
                        CallFunction(ev->Get(&number).c_str(), &ref, 0, 0);
                    }
                }
                number--;
            } while (number > to);
        }
    }
}
