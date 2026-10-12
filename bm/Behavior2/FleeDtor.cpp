// FleeBehavior::~FleeBehavior (SQZ.EXE 0x404040, vtable 0x49d15c).
#include "Activity/Activity.h"
struct FleeBehavior : Behavior {
    Activity *current;                                   // 0x0c
    int mode;                                            // 0x10
    StopActivity *stop;                                  // 0x14
    TurnTargetActivity *walk;                            // 0x18
    virtual ~FleeBehavior();
    virtual int Execute(int dt, int unused);
};
FleeBehavior::~FleeBehavior()
{
    delete walk;
    walk = 0;
    delete stop;
    stop = 0;
}

// SimpleFlyBehavior::~SimpleFlyBehavior (SQZ.EXE 0x4034d0, vtable 0x49d10c): one owned FlyTargetActivity at 0x14.
struct FlyActivity : Activity {
    int pitch, turn;                                     // 0x2ac, 0x2b0
    ~FlyActivity();                                      // 0x405380
};
struct FlyTargetActivity : FlyActivity {
    AV3b target;                                         // 0x2b4 (dtor 0x401660)
    virtual int StepAir(AAirControl *out);               // 0x401c60
};
struct SimpleFlyBehavior : Behavior {
    Activity *current;                                   // 0x0c
    int mode;                                            // 0x10
    FlyTargetActivity *fly;                              // 0x14
    virtual ~SimpleFlyBehavior();
    virtual int Execute(int dt, int unused);
};
SimpleFlyBehavior::~SimpleFlyBehavior()
{
    delete fly;
    fly = 0;
}
