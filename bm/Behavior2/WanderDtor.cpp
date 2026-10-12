// WanderBehavior::~WanderBehavior (SQZ.EXE 0x4025d0): deletes its three activities, then the Behavior base (0x403b50).
#include "Activity/Activity.h"
struct WanderBehavior : Behavior {
    Activity *current;                                   // 0x0c
    int pause, mode;                                     // 0x10, 0x14
    StopActivity *idle;                                  // 0x18
    TurnTargetActivity *walk;                            // 0x1c
    StopActivity *spare;                                 // 0x20
    virtual ~WanderBehavior();                           // 0x4025d0
    virtual int Execute(int dt, int unused);
};
WanderBehavior::~WanderBehavior()
{
    delete idle;
    idle = 0;
    delete walk;
    walk = 0;
    delete spare;
    spare = 0;
}
