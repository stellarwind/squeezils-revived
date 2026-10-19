// SqueezilSoul::Slip (0x453580): self-contained (vector, matrix and contact locals with out-of-line dtors).
#include <math.h>
#include "compat41.h"

struct Matrix16;

struct Vector3f {
    float x, y, z;
    Vector3f() {}
    Vector3f(const float *v) { x = v[0]; y = v[1]; z = v[2]; }
    Vector3f(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    Vector3f(const Vector3f &o) { x = o.x; y = o.y; z = o.z; }
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    Vector3f operator*(float s) const;      // 0x401b60
    Vector3f operator+(const Vector3f &o) const;    // 0x403070
    Vector3f Cross(const Vector3f &o) const;    // 0x41a350
    float Length() const;                   // 0x47d5f0
    float Normalize();                      // 0x47d630
};

struct Matrix {                             // the body's matrix
    float m[16];
};

struct Matrix16 {                           // 64 bytes
    float v[16];
    Matrix16() {}
    Matrix16(const Matrix16 &o);            // 0x4099e0
    Matrix16(const Matrix &o)
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
    }
    ~Matrix16();                            // 0x402310
    Vector3f &Row(int i) { return *(Vector3f *)&v[i * 4]; }
    void SetRotation(const Vector3f &axis, float radians);     // 0x47ddc0
    void SetRows(const Vector3f &right, const Vector3f &up, const Vector3f &forward, const Vector3f &position)
    {
        Row(0) = right; v[3] = 0.0f;
        Row(1) = up; v[7] = 0.0f;
        Row(2) = forward; v[11] = 0.0f;
        Row(3) = position; v[15] = 1.0f;
    }
    void SetTranslation(const Vector3f &p); // 0x47e330
};

Vector3f Transform(const Vector3f &p, const Matrix16 &m);    // 0x47e1e0 (cdecl)

inline float Dot(const Vector3f &a, const Vector3f &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

struct Contact {                            // 0x48 bytes
    int face, second;
    Matrix16 frame;                         // 0x8
    ~Contact();                             // 0x4031e0
};

struct Clock {
    unsigned long FrameMilliseconds();      // 0x41ec00
};
Clock *GetClock();                          // 0x4206d0

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix *GetMatrix();            // slot 3, +0xc
    virtual float   *Direction();           // slot 4, +0x10
    virtual float Speed();                  // slot 5, +0x14
    virtual float Mass();                   // slot 6, +0x18
    virtual Contact *GetContact();          // slot 7, +0x1c
    virtual void b8();
    virtual void SetMatrix(Matrix16 *m);    // slot 9, +0x24
    virtual void SetVelocity(Vector3f *dir, float speed);  // slot 10, +0x28
    virtual void SetAngles(float x, float yaw, float z);   // slot 11, +0x2c
    virtual void GetAngles(float *x, float *yaw, float *z);    // slot 12, +0x30
    virtual void b13();
    virtual void SetContact(Contact *c);    // slot 14, +0x38
};

struct LeverMotion {
    float speedCutoff;                      // 0x00
    float distanceCutoff;                   // 0x04
    float turnAccel;                        // 0x08
    float turnDecel;                        // 0x0c
    float turnRate;                         // 0x10
    float pad14, pad18;
    float frictionScale;                    // 0x1c
    float controlSpeed;                     // 0x20
    float leverArm;                         // 0x24
    float leverWeightFactor;                // 0x28
};

struct GroundControl {
    float forward;                          // 0x00
    float turn;                             // 0x04
    LeverMotion lever;                      // 0x08
};

struct SqueezilPhysics {
    float gravity[3];                       // 0x00
    char  pad0c[0x18 - 0xc];
    float gravityStrength;                  // 0x18
    float kineticFriction;                  // 0x1c
};

struct DirectionHistory {                   // the last few directions (+0x10)
    int size;                               // 0x00
    char pad04[0x18 - 4];
    int added;                              // 0x18
    Vector3f *Add(Vector3f *v);             // 0x456f40
    void Clear();                           // 0x456ef0
};

enum { SS_STAND = 0, SS_HOP = 14 };

const float kRadiansPerDegree = 0.017453292f;

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual void s18(); virtual void s19(); virtual void s20();
    virtual void ResetSurface();            // slot 21, +0x54
    virtual void s22();
    virtual int  CanEnter(int state);       // slot 23, +0x5c
    virtual void SetState(int state, int blend);   // slot 24, +0x60
    virtual int  GetState();                // slot 25, +0x64
    virtual void s26(); virtual void s27();
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31();
    virtual void RotateOnSurface(Matrix16 *m, Vector3f *dir, Contact *c, float radians);   // slot 32, +0x80
    virtual void s33();
    virtual void MoveOnGround(float seconds);                      // slot 34, +0x88
    virtual void StartTumble();                                    // slot 35, +0x8c
    char pad04[0x10 - 4];
    DirectionHistory directions;            // 0x10
    char pad2c[0x48 - 0x2c];
    SqueezilPhysics *physics;               // 0x48
    char pad4c[0x5c - 0x4c];
    float leverSpin;                        // 0x5c

    float SpeedStep(float current, float accel, float decel, float target, float seconds);   // 0x451630
    void Slip(GroundControl *c, int next);
};

// @00453580 Sliding on a slope: the slope's pull turns the squeezil like a lever (leverSpin), friction (FrictionScale times
// KineticFriction) and gravity change the velocity, and the slide stops when it is slow and has not really got anywhere over
// the last few frames. Lifted off the surface (it faces down) it hops off, or tumbles.
void SqueezilSoul::Slip(GroundControl *c, int next)
{
    SqueezilPhysics *p = physics;
    float seconds = GetClock()->FrameMilliseconds() * 0.001f;
    GetState();                             // (result not used)
    Matrix16 m = *Body()->GetMatrix();
    Contact contact = *Body()->GetContact();
    Vector3f gravity(p->gravity);
    float speedCutoff = c->lever.speedCutoff, distanceCutoff = c->lever.distanceCutoff;
    float friction = p->kineticFriction;
    float mass = Body()->Mass();
    Vector3f direction(Body()->Direction());
    float speed = Body()->Speed();
    Vector3f right, up, forward, position, rubbing, momentum, weight, support;
    float angleX, yawRate, angleZ;
    Body()->GetAngles(&angleX, &yawRate, &angleZ);
    up = m.Row(1);
    friction = c->lever.frictionScale * friction;
    float into = Dot(gravity, up);
    float sign = into < 0.0f ? -1.0f : 1.0f;

    if (sign == 1.0f || into > -0.02) {
        // not standing on anything: lift off a little and leave the surface
        m.SetTranslation(m.Row(3) + m.Row(1) * 0.2f);
        Body()->SetMatrix(&m);
        if (CanEnter(SS_HOP)) {
            SetState(SS_HOP, 0);
            ResetSurface();
        } else {
            StartTumble();
        }
        return;
    }

    if (c->lever.controlSpeed != 0.0f) {
        float turnTarget = c->turn * c->lever.turnRate;
        if (fabs(turnTarget) <= 0.1) {
            // no turn wish: the slope's pull turns the squeezil about its feet
            Vector3f side = gravity.Cross(up);
            side.Normalize();
            Vector3f downhill = side.Cross(up);
            float pull = Dot(downhill, gravity);
            float inertia = Body()->Mass() * c->lever.leverWeightFactor * p->gravityStrength * (c->lever.leverArm * c->lever.leverArm);
            right = m.Row(0);
            float sideways = Dot(downhill, right);
            leverSpin += sideways * pull / inertia * seconds;
            forward = m.Row(2);
            if (Dot(downhill, forward) < 0.0f) {
                float k = fabs(sideways);
                if (k < 0.1f)
                    k = 0.1f;
                leverSpin *= k;
            }
        }
        float step = SpeedStep(yawRate, c->lever.turnAccel, c->lever.turnDecel, turnTarget, seconds);
        float control = speed / c->lever.controlSpeed;
        if (control > 1.0f)
            control = 1.0f;
        yawRate += control * step;
        Matrix16 turn;
        turn.SetRotation(up, yawRate * seconds * kRadiansPerDegree);
        direction = Transform(direction, turn);
    }
    float spin = leverSpin + yawRate;
    if (fabs(spin) > 0.1) {
        Vector3f axis(0.0f, 1.0f, 0.0f);    // (the original turns this vector and throws it away)
        RotateOnSurface(&m, &axis, &contact, spin * seconds * kRadiansPerDegree);
    }
    position = m.Row(3);
    right = m.Row(0);
    up = m.Row(1);
    forward = m.Row(2);

    // the new velocity: the old one plus gravity, the surface pushing back, and friction against the movement
    float pressing = fabs(mass * into);
    support = up * pressing;
    rubbing = direction * -(friction * pressing);
    weight = gravity * mass;
    momentum = direction * speed;
    Vector3f before = momentum + weight + support;
    Vector3f after = before + rubbing;
    float newSpeed;
    if (Dot(after, before) < 0.0f) {
        newSpeed = 0.0f;                    // friction turned it round: it stops
    } else {
        newSpeed = after.Normalize();
        Vector3f recent = *directions.Add(&after);
        if (directions.added >= directions.size) {
            float distance = recent.Length();
            if (speedCutoff > newSpeed && distanceCutoff > distance)
                newSpeed = 0.0f;
        }
    }
    m.SetRows(right, up, forward, position);
    Body()->SetMatrix(&m);
    Body()->SetContact(&contact);
    Body()->SetAngles(angleX, yawRate, angleZ);
    if (newSpeed <= 0.01f) {
        Body()->SetVelocity(&direction, 0.0f);
        SetState(CanEnter(next) ? next : SS_STAND, 1);
        directions.Clear();
        leverSpin = 0.0f;
    } else {
        Body()->SetVelocity(&after, newSpeed);
        MoveOnGround(seconds);
    }
}
