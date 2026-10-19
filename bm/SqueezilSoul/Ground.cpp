// SqueezilSoul::GroundMove (0x4540c0): self-contained (vector, matrix and contact locals with out-of-line dtors).
// 1 spare declaration (symbol count, see padscan.sh)
void PadDecl1();
#include <math.h>
#include "compat41.h"

struct Matrix16;

struct Vector3f {
    float x, y, z;
    Vector3f() {}
    Vector3f(const float *v) { x = v[0]; y = v[1]; z = v[2]; }
    Vector3f(const Matrix16 *m, int row);
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    Vector3f operator*(float s) const;      // 0x401b60
    Vector3f operator+(const Vector3f &o) const;    // 0x403070
    float Normalize();                      // 0x47d630
};

struct Matrix16 {                           // 64 bytes
    float v[16];
    Matrix16() {}
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
};

inline Vector3f::Vector3f(const Matrix16 *m, int row) { x = m->v[row * 4]; y = m->v[row * 4 + 1]; z = m->v[row * 4 + 2]; }

struct Contact {                            // 0x48 bytes
    int a, b;
    Matrix16 frame;                         // 0x8
    ~Contact();                             // 0x4031e0
};

struct Clock {
    unsigned long FrameMilliseconds();      // 0x41ec00
};
Clock *GetClock();                          // 0x4206d0

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
    virtual void SetAngles(float x, float yaw, float z);   // slot 11, +0x2c
    virtual void GetAngles(float *x, float *yaw, float *z);    // slot 12, +0x30
    virtual void b13();
    virtual void SetContact(Contact *c);    // slot 14, +0x38
    virtual void b15(); virtual void b16(); virtual void b17(); virtual void b18(); virtual void b19();
    virtual int  GetMode();                 // slot 20, +0x50
    virtual void SetMode(int clip, int blend, int flag);   // slot 21, +0x54
    virtual void b22(); virtual void b23();
    virtual void SetClipSpeed(float speed); // slot 24, +0x60
};

struct GroundControl {
    float forward;                          // 0x00 -1 (back) .. 1 (forward)
    float turn;                             // 0x04
    float acceleration;                     // 0x08
    float deceleration;                     // 0x0c
    float speed;                            // 0x10
    float turnAccel;                        // 0x14
    float turnDecel;                        // 0x18
    float turnRate;                         // 0x1c
};

struct SqueezilPhysics {
    char  pad[0x2c];
    float walkSpeed;                        // 0x2c (walk.speed)
};

enum { SS_STAND = 0, SS_RUN = 4, SS_SLIP = 6, SS_NONE = 24 };
enum { MODE_WALK = 2, MODE_RUN = 3 };

const float kRadiansPerDegree = 0.017453292f;

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
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31();
    virtual void RotateOnSurface(Matrix16 *m, Vector3f *dir, Contact *c, float radians);   // slot 32, +0x80
    virtual int  WouldSlip(float push, Matrix16 *m, Contact *c);   // slot 33, +0x84
    virtual void MoveOnGround(float seconds);                      // slot 34, +0x88
    char pad04[0x48 - 4];
    SqueezilPhysics *physics;               // 0x48

    float SpeedStep(float current, float accel, float decel, float target, float seconds);   // 0x451630
    void GroundMove(GroundControl *c, int next);
};

// @004540c0 Standing, walking, running, scampering: turn by the turn wish, speed up or slow down along the facing by the
// wish, slip if the push is more than the grip, then move along the surface. Running backwards plays the walk clip.
void SqueezilSoul::GroundMove(GroundControl *c, int next)
{
    Matrix16 m = *Body()->GetMatrix();
    Contact contact = *Body()->GetContact();
    Vector3f direction(Body()->Direction());
    Vector3f position(&m, 3);
    float speed = Body()->Speed();
    float seconds = GetClock()->FrameMilliseconds() * 0.001f;
    Matrix16 unused1, unused2;
    Vector3f right, up, forward;
    int startState = GetState();
    int backwards = speed < 0.0f;
    if (direction.x * m.v[8] + direction.y * m.v[9] + direction.z * m.v[10] < 0.0f)
        backwards = !backwards;
    if (next != SS_NONE && CanEnter(next) && (next != SS_STAND || speed <= 0.01f))
        SetState(next, 0);

    float angleX, yawRate, angleZ;
    Body()->GetAngles(&angleX, &yawRate, &angleZ);
    yawRate += SpeedStep(yawRate, c->turnAccel, c->turnDecel, c->turn * c->turnRate, seconds);
    if (fabs(yawRate) > 0.1)
        RotateOnSurface(&m, &direction, &contact, seconds * yawRate * kRadiansPerDegree);
    right = m.Row(0);
    up = m.Row(1);
    forward = m.Row(2);
    if (speed <= 0.01f)
        direction = forward * (c->forward < 0.0f ? -1.0f : 1.0f);
    else
        direction = forward * (backwards ? -1.0f : 1.0f);

    float along = (direction.x * forward.x + direction.y * forward.y + direction.z * forward.z) * speed;
    float change = SpeedStep(along, c->acceleration, c->deceleration, c->forward * c->speed, seconds);
    if (WouldSlip(change / seconds, &m, &contact) && CanEnter(SS_SLIP)) {
        SetState(SS_SLIP, 0);
        return;
    }
    if (fabs(change) > 0.01 || speed > 0.01f) {
        Vector3f push = forward * change;
        Vector3f velocity = direction * speed;
        velocity = velocity + push;
        speed = velocity.Normalize();
        direction = velocity;
    }
    m.SetRows(right, up, forward, position);
    Body()->SetMatrix(&m);
    Body()->SetVelocity(&direction, speed);
    Body()->SetAngles(angleX, yawRate, angleZ);
    Body()->SetContact(&contact);
    if (GetState() == SS_RUN) {
        int nowBackwards = direction.x * forward.x + direction.y * forward.y + direction.z * forward.z < 0.0f;
        if (speed < 0.0f)
            nowBackwards = !nowBackwards;
        if (backwards != nowBackwards || startState != SS_RUN)
        {
            if (nowBackwards)
                Body()->SetMode(MODE_WALK, 0, 0);
            else
                Body()->SetMode(MODE_RUN, 0, 0);
        }
    }
    float clipSpeed;
    if (GetState() == SS_RUN && Body()->GetMode() == MODE_WALK)
        clipSpeed = physics->walkSpeed;
    else
        clipSpeed = c->speed;
    Body()->SetClipSpeed(clipSpeed);
    MoveOnGround(seconds);
}
