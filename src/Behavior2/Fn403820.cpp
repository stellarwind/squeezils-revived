// 0x403820: destructor of a behavior holding a turn-to activity (0x18) and two stop activities (0x1c, 0x14).
#include "Activity/Activity.h"
struct ThreeActBehavior : Behavior {
    Activity *current;                                   // 0x0c
    int pause;                                           // 0x10
    StopActivity *stopA;                                 // 0x14
    TurnTargetActivity *walk;                            // 0x18
    StopActivity *stopB;                                 // 0x1c
    virtual ~ThreeActBehavior();                         // 0x403820
    virtual int Execute(int dt, int unused);
};
ThreeActBehavior::~ThreeActBehavior()
{
    delete walk;
    walk = 0;
    delete stopB;
    stopB = 0;
    delete stopA;
    stopA = 0;
}
