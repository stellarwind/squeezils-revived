// WanderBehavior::Execute (0x4028a0): pick a target, walk to it, pause, repeat.
#include "Behavior2/Activity.h"

struct WanderSub {                                       // the walk / idle sub-behaviours
    virtual int Run(int dt);                             // slot 0 (+0)
    virtual int Execute(int dt);                         // slot 1 (+4)
    char pad[0x2b4 - 4];
    unsigned flags;                                      // 0x2b4
    void Prepare();                                      // 0x401270
};
struct WanderSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23(); virtual void s24();
    virtual int State();                                 // slot 25 (+0x64)
};
struct WanderBehavior {
    virtual void s0();
    virtual int Execute(int dt, int unused);             // slot 1
    void *body;                                          // 0x04
    WanderSoul *soul;                                    // 0x08
    WanderSub *current;                                  // 0x0c
    int pause;                                           // 0x10
    int mode;                                            // 0x14
    WanderSub *idle;                                     // 0x18
    WanderSub *walk;                                     // 0x1c
    void PickTarget();                                   // 0x4026e0
};

int WanderBehavior::Execute(int dt, int unused)
{
    soul->State();
    switch (mode) {
    case 0:
        PickTarget();
        current = walk;
        mode++;
        break;
    case 1:
        if (!(walk->flags & 0x7fffffff)) {
            current = idle;
            mode++;
            pause = 50;
        }
        break;
    case 2:
        if (--pause < 0)
            mode = 0;
        break;
    }
    current->Prepare();
    return current->Execute(dt);
}
