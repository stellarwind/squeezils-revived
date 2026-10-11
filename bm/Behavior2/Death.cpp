// DeathBehavior::Execute (0x403310): play the death, wait for it to finish, count the delay down, then move on.
#include "Behavior2/Activity.h"

struct DeathSub {
    virtual int Run(int a);                              // slot 0
    virtual int Execute(int a);                          // slot 1
};
struct DeathSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual int State();                                 // slot 25 (+0x64)
};
struct DeathClock { int Elapsed(); };                    // 0x41ec00
DeathClock *GetClock();                                  // 0x4206d0 (cdecl)

struct DeathBehavior {
    virtual void s0();
    virtual int Execute(int a, int b);                   // slot 1
    void *body;                                          // 0x04
    DeathSoul *soul;                                     // 0x08
    DeathSub *current;                                   // 0x0c
    int mode;                                            // 0x10
    DeathSub *idle;                                      // 0x14
    DeathSub *walk;                                      // 0x18
    float duration;                                      // 0x1c
    int countdown;                                       // 0x20
};

int DeathBehavior::Execute(int a, int b)
{
    switch (mode) {
    case 0:
        current = idle;
        mode++;
        break;
    case 1:
        if (soul->State() == 0x16) {
            countdown = (int)(duration * 1000.0f);
            mode++;
        }
        break;
    case 2:
        if (duration > 0) {
            countdown -= GetClock()->Elapsed();
            if (countdown <= 0) {
                current = walk;
                mode++;
            }
        }
        break;
    case 3:
        if (soul->State() != 0x16)
            mode++;
        break;
    }
    return current->Execute(a);
}
