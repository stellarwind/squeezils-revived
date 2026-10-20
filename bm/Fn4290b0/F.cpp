// 0x4290b0: CycleProp::FireStepMessages - going down from..to, run the script named for each number that has one.
#include "String/String.h"
struct Base { virtual ~Base(); };
struct Target : Base { };
struct Events { virtual int v0(); virtual int Has(int *k); String *Get(int *k); };      // slot 1 (+4); 0x4638b0
struct Ref { int d[6]; Ref(int handle, void *owner); Ref(Target *t, int handle, void *owner); ~Ref(); };   // 0x4207a0 / 0x420900
int RunCommand(const char *name, Ref *r, int a, int b);              // 0x422ce0 (cdecl)
struct Owner;
Owner *__stdcall GetOwner(int a);                                              // 0x420690 (stdcall)
struct Owner { Base *Find(); };                                      // 0x414080
struct CycleProp {
    char pad[0x9c];
    int handle;             // 0x9c
    char pad2[0xac - 0xa0];
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
                        Target *t = dynamic_cast<Target *>(GetOwner(handle)->Find());
                        Ref ref(t, handle, this);
                        { String *name = ev->Get(&number); RunCommand(name->c_str(), &ref, 0, 0); }
                    } else {
                        Ref ref(-1, this);
                        { String *name = ev->Get(&number); RunCommand(name->c_str(), &ref, 0, 0); }
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
                        Target *t = dynamic_cast<Target *>(GetOwner(handle)->Find());
                        Ref ref(t, handle, this);
                        { String *name = ev->Get(&number); RunCommand(name->c_str(), &ref, 0, 0); }
                    } else {
                        Ref ref(-1, this);
                        { String *name = ev->Get(&number); RunCommand(name->c_str(), &ref, 0, 0); }
                    }
                }
                number--;
            } while (number > to);
        }
    }
}
