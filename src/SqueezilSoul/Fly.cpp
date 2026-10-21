// SqueezilSoul::Fly (0x454b40): self-contained (vector, matrix, contact and string values with out-of-line dtors).
#include <string.h>
#pragma intrinsic(strlen)
#include "compat41.h"

class String {
public:
    ~String();                              // 0x4383d0
    const char *Chars();                    // 0x438530
    char *rep;
};

struct Vector3f {
    float x, y, z;
    Vector3f() {}
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    Vector3f operator*(float s) const;      // 0x401b60
    Vector3f operator+(const Vector3f &o) const;    // 0x403070
    float Length() const;                   // 0x47d5f0
    float Normalize();                      // 0x47d630
};

struct Matrix {                             // the body's matrix
    float m[16];
};

struct Matrix16 {                           // 64 bytes
    float v[16];
    Matrix16() {}
    ~Matrix16();                            // 0x402310
    Matrix16 &operator=(const Matrix16 &o);    // 0x402340
    Matrix16 &operator=(const Matrix &o)
    {
        v[0] = o.m[0];
        v[1] = o.m[1];
        v[2] = o.m[2];
        v[3] = o.m[3];
        v[4] = o.m[4];
        v[5] = o.m[5];
        v[6] = o.m[6];
        v[7] = o.m[7];
        v[8] = o.m[8];
        v[9] = o.m[9];
        v[10] = o.m[10];
        v[11] = o.m[11];
        v[12] = o.m[12];
        v[13] = o.m[13];
        v[14] = o.m[14];
        v[15] = o.m[15];
        return *this;
    }
    Vector3f &Row(int i) { return *(Vector3f *)&v[i * 4]; }
    void SetRows(const Vector3f &right, const Vector3f &up, const Vector3f &forward, const Vector3f &position)
    {
        v[0] = right.x; v[1] = right.y; v[2] = right.z; v[3] = 0.0f;
        v[4] = up.x; v[5] = up.y; v[6] = up.z; v[7] = 0.0f;
        v[8] = forward.x; v[9] = forward.y; v[10] = forward.z; v[11] = 0.0f;
        v[12] = position.x; v[13] = position.y; v[14] = position.z; v[15] = 1.0f;
    }
    void SetTranslation(const Vector3f &p); // 0x47e330
};

struct Contact {                            // 0x48 bytes
    int face, second;
    Matrix16 frame;                         // 0x8
    Contact() : face(-1), second(0) {}
    ~Contact();                             // 0x4031e0
};

struct Clock {
    unsigned long FrameMilliseconds();      // 0x41ec00
};
Clock *GetClock();                          // 0x4206d0

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix *GetMatrix();            // slot 3, +0xc
    virtual Vector3f *Direction();          // slot 4, +0x10
    virtual float Speed();                  // slot 5, +0x14
    virtual void b6();
    virtual Contact *GetContact();          // slot 7, +0x1c
    virtual void b8();
    virtual void SetMatrix(Matrix16 *m);    // slot 9, +0x24
    virtual void SetVelocity(Vector3f *dir, float speed);  // slot 10, +0x28
    virtual void b11(); virtual void b12(); virtual void b13();
    virtual void SetContact(Contact *c);    // slot 14, +0x38
    virtual void b15(); virtual void b16(); virtual void b17();
    virtual void GetOrientation(float *pitch, float *yaw, float *roll);    // slot 18, +0x48
};

struct SqueezilSoul;

struct NavSurfaces {
    int FlightHits(Vector3f *position, Vector3f *right, Vector3f *up, Vector3f *forward, Vector3f *direction, float distance,
                   Vector3f *hit, Vector3f *newRight, Vector3f *newUp, Vector3f *newForward, Contact *landing);   // 0x4108e0
    int Bounces(Contact *c);                // 0x40f610
    int FlyThrough(Contact *c);             // 0x40f8d0
    String ActivateScript(Contact *c);      // 0x40f9b0
};

struct Place {
    NavSurfaces *Surfaces();                // 0x439b60
};

struct ScriptCaller {
    int data[3];
    void InitSoul(SqueezilSoul *soul, int actorIndex, void *prop);     // 0x420900
};
#include "Script/CallFunction.h"

struct Simulation {
    int FindIdBySoul(void *soul);           // 0x4140b0
};
Simulation *GetSimulation();                // 0x420690

extern float gFloorLevel;                   // 0x4c7ac8
extern String gFloorLevelScript;            // 0x4c7acc

struct AirControl {
    float pitch;                            // 0x00 -1 (climb) .. 1 (dive), or kNoInput
    char  pad04[0x2c - 4];
    float speedFactor;                      // 0x2c
    float pad30;
    float dragZMult;                        // 0x34
    float pad38;
    float dragYMult;                        // 0x3c
    float pad40;
    float dragXMult;                        // 0x44
    float pad48;
    float liftMult;                         // 0x4c
    float aoaAdjustMult;                    // 0x50
    float stallAdjustMult;                  // 0x54
    float tuckLiftFactor;                   // 0x58
    float climbLiftFactor;                  // 0x5c
};

struct SqueezilPhysics {
    Vector3f gravity;                       // 0x00
};

enum { SS_FLY = 17, SS_LAND = 19, SS_NONE = 24 };

#define kNoInput 1000.0f

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22();
    virtual int  CanEnter(int state);       // slot 23, +0x5c
    virtual void SetState(int state, int blend);   // slot 24, +0x60
    virtual int  GetState();                // slot 25, +0x64
    virtual void s26(); virtual void s27();
    virtual void s28(); virtual void s29(); virtual void s30();
    virtual void Steer(AirControl *c, float levelTo, Matrix16 *m, float seconds);   // slot 31, +0x7c
    char pad04[0x30 - 4];
    float pitchTarget;                      // 0x30
    char pad34[0x48 - 0x34];
    SqueezilPhysics *physics;               // 0x48
    char pad4c[0x70 - 0x4c];
    Place *place;                           // 0x70

    void Fly(AirControl *c, int next);
};

// The floor level: a squeezil going below it from above is held at it, and the level's script runs
inline void CheckFloorLevel(SqueezilSoul *soul, Vector3f *position, float oldY)
{
    if (position->y < gFloorLevel && oldY > gFloorLevel) {
        position->y = gFloorLevel;
        if (strlen(gFloorLevelScript.Chars()) != 0) {
            ScriptCaller caller;
            caller.InitSoul(soul, GetSimulation()->FindIdBySoul(soul), 0);
            CallFunction(gFloorLevelScript.Chars(), &caller, 0, 0);
        }
    }
}

// The script function a surface type names, run by the squeezil that touched it
inline void RunScript(SqueezilSoul *soul, String &name)
{
    if (strlen(name.Chars()) != 0) {
        ScriptCaller caller;
        caller.InitSoul(soul, GetSimulation()->FindIdBySoul(soul), 0);
        CallFunction(name.Chars(), &caller, 0, 0);
    }
}

// @00454b40 One frame of flight (gliding, diving, and every other kind of flight through the helpers above): steer, then the
// velocity changes by gravity, lift (along up, from the forward speed) and drag along each of the body's axes; flying
// backwards keeps only the sideways and vertical parts. A flight that reaches a surface lands there (SS_LAND), unless the
// surface's type pushes it back or runs a script instead. The pitch target makes the nose follow the velocity.
void SqueezilSoul::Fly(AirControl *c, int next)
{
    Vector3f right, up, forward, at, position, direction, change, newVelocity, step;
    Matrix16 m;
    Contact contact, landing;
    Vector3f velocity, dragZ, dragY, dragX, liftForce;
    if (next != SS_NONE && CanEnter(next))
        SetState(next, 0);
    float seconds = GetClock()->FrameMilliseconds() * 0.001f;
    m = *Body()->GetMatrix();
    position = m.Row(3);
    float speed = Body()->Speed();
    direction = *Body()->Direction();
    contact = *Body()->GetContact();       // (never used)
    Steer(c, pitchTarget, &m, seconds);
    right = m.Row(0);
    up = m.Row(1);
    forward = m.Row(2);
    float pitch, yaw, roll;
    Body()->GetOrientation(&pitch, &yaw, &roll);

    velocity = direction * speed;
    float along = velocity.x * forward.x + velocity.y * forward.y + velocity.z * forward.z;
    float rise = velocity.x * up.x + velocity.y * up.y + velocity.z * up.z;
    float side = velocity.x * right.x + velocity.y * right.y + velocity.z * right.z;
    float sideSign = side < 0.0f ? -1.0f : 1.0f;
    dragX = right * -(side * sideSign * c->dragXMult * sideSign);
    float riseSign = rise < 0.0f ? -1.0f : 1.0f;
    dragY = up * -(rise * riseSign * c->dragYMult * riseSign);
    float alongSign = along < 0.0f ? -1.0f : 1.0f;
    along = alongSign * along;
    dragZ = forward * -(along * c->dragZMult * alongSign);
    float lift = c->liftMult * along * alongSign;
    if (c->pitch != kNoInput)
        lift *= c->pitch > 0.0f ? c->tuckLiftFactor : c->climbLiftFactor;
    liftForce = up * lift;
    change = (liftForce + dragZ + dragY + dragX + physics->gravity) * seconds;
    newVelocity = velocity + change;

    float newAlong = newVelocity.x * forward.x + newVelocity.y * forward.y + newVelocity.z * forward.z;
    float newAlongSign = newAlong < 0.0f ? -1.0f : 1.0f;
    if (newAlongSign == -1.0f) {
        float a = newVelocity.x * right.x + newVelocity.y * right.y + newVelocity.z * right.z;
        rise = newVelocity.x * up.x + newVelocity.y * up.y + newVelocity.z * up.z;
        newVelocity = right * a + up * rise;
    }
    direction = newVelocity;
    float newSpeed = direction.Normalize();
    step = newVelocity * seconds * c->speedFactor;

    int landed = place->Surfaces()->FlightHits(&position, &right, &up, &forward, &direction, step.Length(), &at, &right, &up,
                                               &forward, &landing);
    if (landed && place->Surfaces()->Bounces(&landing)) {
        landed = 0;
        step = step * -1.0f;
    }
    if (landed && place->Surfaces()->FlyThrough(&landing)) {
        landed = 0;
        RunScript(this, place->Surfaces()->ActivateScript(&landing));
    }
    if (landed) {
        m.SetRows(right, up, forward, at);
        Body()->SetContact(&landing);
        Body()->SetMatrix(&m);
        SetState(SS_LAND, 0);
        pitchTarget = kNoInput;
        CheckFloorLevel(this, &at, position.y);     // (after the matrix is set, so the hold has no effect)
        return;
    }
    at = position + step;
    CheckFloorLevel(this, &at, position.y);
    if (newSpeed > 0.01f) {
        rise = -(rise / newSpeed) * c->aoaAdjustMult;
        if (GetState() == SS_FLY && newAlong < 0.0f)
            rise = rise - c->stallAdjustMult / newSpeed * newAlong;
    } else {
        rise = 0.0f;
    }
    pitchTarget = rise + pitch;
    m.SetTranslation(at);
    Body()->SetMatrix(&m);
    Body()->SetVelocity(&direction, newSpeed);
}
