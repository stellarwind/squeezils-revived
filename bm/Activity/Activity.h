// Activity (base class of the robo squeezils' activities), constructor at SQZ.EXE 0x401000.
#pragma once
#include "compat41.h"

struct AV3 {                                    // Vector3f: empty ctor, out-of-line dtor 0x401660 and assignment 0x401670
    float x, y, z;
    AV3() {}
    ~AV3();
    AV3 &operator=(const AV3 &o);
};
struct APhysical {
    AV3 gravity; AV3 down; float gravityStrength, kineticFriction, staticFriction;
    APhysical &operator=(const APhysical &o)
    {
        gravity.x = o.gravity.x; gravity.y = o.gravity.y; gravity.z = o.gravity.z;
        down = o.down;
        gravityStrength = o.gravityStrength; kineticFriction = o.kineticFriction; staticFriction = o.staticFriction;
        return *this;
    }
};
struct AGround { float v[6]; };
struct AFlight { float v[23]; };
struct AJump { float v[10]; };
struct ALever { float v[11]; };
struct APhysics {                               // SqueezilPhysics (0x2a0), destructor out of line (0x405260)
    APhysical physical;
    AGround walk, run, scamper, skid;
    AFlight fly, dive, tumble;
    AJump jump, hop, skihop;
    ALever attack, slip;
    float t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13;
    APhysics() {}
    ~APhysics();
};
struct ASoul { char pad[0x48]; APhysics *physics; };
struct ABody;

struct AGroundControl; struct AAirControl;
struct Activity {
    virtual int StepAir(AAirControl *out) = 0;
    virtual int StepGround(AGroundControl *out) = 0;
    ABody *body;
    ASoul *soul;
    APhysics physics;
    Activity(ABody *b, ASoul *s);               // 0x401000
    void RefreshPhysics();                      // 0x401270
    ~Activity();                                // 0x405210
};
struct WalkActivity : Activity {
    WalkActivity(ABody *b, ASoul *s) : Activity(b, s) {}
    ~WalkActivity();                            // 0x405320
    virtual int StepAir(AAirControl *out);      // 0x405200
};
struct MoveActivity : WalkActivity {
    int direction, turn;
    float turnAmount;
    int gait;
    MoveActivity(ABody *b, ASoul *s) : WalkActivity(b, s), direction(1), turn(0), turnAmount(0), gait(3) {}
    ~MoveActivity();                            // 0x401690
    virtual int StepGround(AGroundControl *out);   // 0x401430
};
struct AV3b { float x, y, z; AV3b() {} ~AV3b(); };   // dtor 0x401660
struct TurnTargetActivity : MoveActivity {
    AV3b target;
    float cosAngle;
    TurnTargetActivity(ABody *b, ASoul *s);     // 0x401510
    virtual int StepGround(AGroundControl *out);   // 0x401700
};
struct DieActivity : WalkActivity {
    int done;
    DieActivity(ABody *b, ASoul *s);    // 0x401e60
    virtual int StepGround(AGroundControl *out);                           // 0x401ef0
};
struct RegenActivity : WalkActivity {
    RegenActivity(ABody *b, ASoul *s);            // 0x401f50
    virtual int StepGround(AGroundControl *out);                           // 0x401fd0
};
struct StopActivity : WalkActivity {
    StopActivity(ABody *b, ASoul *s) : WalkActivity(b, s) {}
    virtual int StepGround(AGroundControl *out);                           // 0x401be0
};
struct Behavior {
    Behavior(ABody *b, ASoul *s);                        // 0x4022f0
    virtual ~Behavior();                                 // 0x403b50 (scalar deleting 0x402320)
    virtual int Execute(int dt, int unused) = 0;
    ABody *body; ASoul *soul;
};
