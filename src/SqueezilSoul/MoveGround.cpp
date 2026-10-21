// Declaration padding preserves VC4 code after sharing script dispatch.
void DispatchPad000();
void DispatchPad001();
void DispatchPad002();
void DispatchPad003();
void DispatchPad004();
void DispatchPad005();
void DispatchPad006();
void DispatchPad007();
void DispatchPad008();
void DispatchPad009();
void DispatchPad010();
void DispatchPad011();
void DispatchPad012();
void DispatchPad013();
void DispatchPad014();
void DispatchPad015();
// SqueezilSoul::MoveOnGround (0x451fd0): self-contained (vector, matrix and contact locals with out-of-line dtors).
#include <string.h>
#include <math.h>
#pragma intrinsic(strlen, acos)
#include "compat41.h"

class String {
public:
    const char *Chars();                    // 0x438530
    char *rep;
};

struct Matrix16;

struct Vector3f {
    float x, y, z;
    Vector3f(const float *v) { x = v[0]; y = v[1]; z = v[2]; }
    Vector3f(const Matrix16 *m, int row);
    Vector3f(const Vector3f &o) { x = o.x; y = o.y; z = o.z; }
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    Vector3f operator*(float s) const;      // 0x401b60
    Vector3f operator+(const Vector3f &o) const;    // 0x403070
    float Normalize();                      // 0x47d630
};

struct Matrix16 {                           // 64 bytes
    float v[16];
    Matrix16(const Matrix16 &o)
    {
        v[0] = o.v[0];
        v[1] = o.v[1];
        v[2] = o.v[2];
        v[3] = o.v[3];
        v[4] = o.v[4];
        v[5] = o.v[5];
        v[6] = o.v[6];
        v[7] = o.v[7];
        v[8] = o.v[8];
        v[9] = o.v[9];
        v[10] = o.v[10];
        v[11] = o.v[11];
        v[12] = o.v[12];
        v[13] = o.v[13];
        v[14] = o.v[14];
        v[15] = o.v[15];
    }
    ~Matrix16();                            // 0x402310
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

inline Vector3f::Vector3f(const Matrix16 *m, int row) { x = m->v[row * 4]; y = m->v[row * 4 + 1]; z = m->v[row * 4 + 2]; }

inline float Dot(const Vector3f &a, const Vector3f &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

struct Contact {                            // 0x48 bytes
    int face, second;
    Matrix16 frame;                         // 0x8
    ~Contact();                             // 0x4031e0
};

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix16 *GetMatrix();          // slot 3, +0xc
    virtual float   *Direction();           // slot 4, +0x10
    virtual float Speed();                  // slot 5, +0x14
    virtual void b6();
    virtual Contact *GetContact();          // slot 7, +0x1c
    virtual void b8();
    virtual void SetMatrix(Matrix16 *m);    // slot 9, +0x24
    virtual void SetVelocity(Vector3f *dir, float speed);  // slot 10, +0x28
    virtual void b11(); virtual void b12(); virtual void b13();
    virtual void SetContact(Contact *c);    // slot 14, +0x38
};

struct SqueezilSoul;

struct NavSurfaces {
    int MoveAlong(SqueezilSoul *soul, Vector3f *position, Vector3f *right, Vector3f *up, Vector3f *forward,
                  Vector3f *direction, float distance, Contact *contact, Vector3f *newPosition, Vector3f *newRight,
                  Vector3f *newUp, Vector3f *newForward, Vector3f *newDirection, Contact *newContact);   // 0x411120
    int Bounces(Contact *c);                // 0x40f610
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

struct SqueezilPhysics {
    char     pad[0xc];
    Vector3f gravity;                       // 0xc
    char     pad18[0x284 - 0x18];
    float    cornerAngle;                   // 0x284
};

enum { SS_JUMP = 12, SS_HOP = 14, SS_SKIHOP = 15 };

const float kDegreesPerRadian = 57.29578f;

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual void s18(); virtual void s19(); virtual void s20();
    virtual void ResetSurface();            // slot 21, +0x54
    virtual void UpdateSurface(Contact contact);   // slot 22, +0x58
    virtual int  CanEnter(int state);       // slot 23, +0x5c
    virtual void SetState(int state, int blend);   // slot 24, +0x60
    virtual void s25(); virtual void s26(); virtual void s27();
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31(); virtual void s32();
    virtual int  WouldSlip(float push, Matrix16 *m, Contact *c);   // slot 33, +0x84
    virtual void s34();
    virtual void StartTumble();             // slot 35, +0x8c
    char pad04[0x48 - 4];
    SqueezilPhysics *physics;               // 0x48
    char pad4c[0x70 - 0x4c];
    Place *place;                           // 0x70

    void MoveOnGround(float seconds);
};

// @00451fd0 Carries the body `seconds` along the surface at its speed. Over a sharp corner (more than CornerAngle) where it
// would slip, it either goes off the edge (moving into the new face) or bounces back (from flat ground onto a steep face).
// Off the surface it hops, jumps or tumbles into the air.
void SqueezilSoul::MoveOnGround(float seconds)
{
    Contact contact = *Body()->GetContact();
    Vector3f direction(Body()->Direction());
    float speed = Body()->Speed();
    Matrix16 m = *Body()->GetMatrix();
    Vector3f position(&m, 3);
    Vector3f oldPosition = position;
    Vector3f right(&m, 0);
    Vector3f up(&m, 1);
    Vector3f forward(&m, 2);
    float distance = speed * seconds;
    float sign = Dot(direction, forward) < 0.0f ? -1.0f : 1.0f;
    int moved = place->Surfaces()->MoveAlong(this, &position, &right, &up, &forward, &direction, distance, &contact,
                                             &position, &right, &up, &forward, &direction, &contact);
    int bounce = 0;
    if (moved && place->Surfaces()->Bounces(&contact))
        bounce = 1;
    if (moved) {
        if (Body()->GetContact()->face != contact.face || Body()->GetContact()->second != contact.second) {
            if (acos(Dot(m.Row(1), up)) * kDegreesPerRadian > physics->cornerAngle && WouldSlip(0.0f, &m, &contact)) {
                if (Dot(*(Vector3f *)Body()->Direction(), up) > 0.0f) {
                    moved = 0;                          // over the edge
                } else {
                    int steep = Dot(physics->gravity * -1.0f, m.Row(1)) > 0.8 && Dot(physics->gravity * -1.0f, up) < 0.3;
                    if (steep)
                        bounce = 1;
                }
            }
        }
    }
    if (bounce) {
        speed = -speed;
        Body()->SetVelocity(&direction, speed);
        return;
    }
    if (!moved) {
        m.SetTranslation(oldPosition + up * 0.2f);
        Body()->SetMatrix(&m);
        if (CanEnter(SS_SKIHOP))
            SetState(SS_SKIHOP, 0);
        else if (CanEnter(SS_HOP))
            SetState(SS_HOP, 0);
        else if (CanEnter(SS_JUMP))
            SetState(SS_JUMP, 0);
        else
            StartTumble();
        return;
    }
    if (position.y < gFloorLevel && oldPosition.y > gFloorLevel) {
        position.y = gFloorLevel;
        if (strlen(gFloorLevelScript.Chars()) != 0) {
            ScriptCaller caller;
            caller.InitSoul(this, GetSimulation()->FindIdBySoul(this), 0);
            CallFunction(gFloorLevelScript.Chars(), &caller, 0, 0);
        }
    }
    m.Row(0) = right; m.v[3] = 0.0f;
    m.Row(1) = up; m.v[7] = 0.0f;
    m.Row(2) = forward; m.v[11] = 0.0f;
    m.Row(3) = position; m.v[15] = 1.0f;
    direction.Normalize();
    Contact previous = *Body()->GetContact();
    if (previous.face != contact.face)
        ResetSurface();
    Body()->SetMatrix(&m);
    Body()->SetVelocity(&direction, speed);
    Body()->SetContact(&contact);
    UpdateSurface(contact);
}
