// SqueezilSoul::Steer (0x451a50): self-contained (vector and matrix locals with out-of-line dtors).
// 13 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
#include "compat41.h"

struct Vector3f {
    float x, y, z;
    Vector3f() {}
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
};

struct Matrix16 {                           // 64 bytes
    float v[16];
    Matrix16() {}
    ~Matrix16();                            // 0x402310
    Matrix16 &operator=(const Matrix16 &o)
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
        return *this;
    }
    void SetIdentity()
    {
        v[0] = 1.0f; v[1] = 0.0f; v[2] = 0.0f; v[3] = 0.0f;
        v[4] = 0.0f; v[5] = 1.0f; v[6] = 0.0f; v[7] = 0.0f;
        v[8] = 0.0f; v[9] = 0.0f; v[10] = 1.0f; v[11] = 0.0f;
        v[12] = 0.0f; v[13] = 0.0f; v[14] = 0.0f; v[15] = 1.0f;
    }
    Vector3f &Row(int i) { return *(Vector3f *)&v[i * 4]; }
    void RotationX(float radians);          // 0x47dcd0
    void RotationY(float radians);          // 0x47dd20
    void RotationZ(float radians);          // 0x47dd70
    Matrix16 operator*(const Matrix16 &o);  // 0x47dea0
    void SetTranslation(const Vector3f &p); // 0x47e330
};

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual float *GetMatrix();             // slot 3, +0xc
    virtual float *Direction();             // slot 4, +0x10
    virtual float Speed();                  // slot 5, +0x14
    virtual void b6(); virtual void b7(); virtual void b8(); virtual void b9(); virtual void b10();
    virtual void SetAngles(float pitchRate, float yawRate, float rollRate);       // slot 11, +0x2c
    virtual void GetAngles(float *pitchRate, float *yawRate, float *rollRate);    // slot 12, +0x30
    virtual void b13(); virtual void b14(); virtual void b15(); virtual void b16();
    virtual void SetOrientation(float pitch, float yaw, float roll);             // slot 17, +0x44
    virtual void GetOrientation(float *pitch, float *yaw, float *roll);          // slot 18, +0x48
};

struct AirControl {
    float pitch;                            // 0x00 -1 (climb) .. 1 (dive), or kNoInput
    float turn;                             // 0x04
    float diveAngle;                        // 0x08
    float climbAngle;                       // 0x0c
    float diveAdjustRate;                   // 0x10
    float turnAccel;                        // 0x14
    float turnDecel;                        // 0x18
    float turnRate;                         // 0x1c
    float bank;                             // 0x20
    float pitchCorrectionRate;              // 0x24
    float controlCutoff;                    // 0x28
    char  pad2c[0x60 - 0x2c];
    float climbPitchFactor;                 // 0x60
};

#define kNoInput 1000.0f
const float kRadiansPerDegree = 0.017453292f;

inline float DotRow2(const float *d, const float *m) { return d[1] * m[9] + d[2] * m[10] + m[8] * d[0]; }

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44

    float SpeedStep(float current, float accel, float decel, float target, float seconds);   // 0x451630
    float AngleStep(float current, float rate, float target, float seconds);                 // 0x451740
    void Steer(AirControl *c, float levelTo, Matrix16 *m, float seconds);
};

// @00451a50 Flying: the turn wish turns (yaw rate eased toward TurnRate), the pitch wish climbs or dives toward
// ClimbAngle/DiveAngle (weaker when slow), levelTo pulls the pitch back, and the roll banks with the turn. Rebuilds the
// matrix from yaw, pitch and roll and hands the rates and angles back to the body.
void SqueezilSoul::Steer(AirControl *c, float levelTo, Matrix16 *m, float seconds)
{
    Vector3f position;
    Matrix16 turn;
    float pitch, yaw, roll;
    Body()->GetOrientation(&pitch, &yaw, &roll);
    float pitchRate, yawRate, rollRate;
    Body()->GetAngles(&pitchRate, &yawRate, &rollRate);

    yawRate += SpeedStep(yawRate, c->turnAccel, c->turnDecel, c->turn * c->turnRate, seconds);
    yaw += yawRate * seconds;
    if (yaw >= 360.0f)
        yaw -= 360.0f;
    if (yaw < 0.0f)
        yaw += 360.0f;

    float pitchStep;
    if (c->pitch != kNoInput) {
        if (c->pitch < 0.0f)
            pitchStep = c->climbAngle * c->pitch;
        else
            pitchStep = c->diveAngle * c->pitch;
        pitchStep = AngleStep(pitch, c->diveAdjustRate, pitchStep, seconds);
        float speed = Body()->Speed();
        float forwardSpeed = DotRow2(Body()->Direction(), Body()->GetMatrix()) * speed;
        if (forwardSpeed < c->controlCutoff) {          // too slow to steer fully: fades out over the last unit of speed
            float control;
            if (c->controlCutoff - 1.0f <= forwardSpeed)
                control = forwardSpeed - c->controlCutoff + 1.0f;
            else
                control = 0.0f;
            pitchStep *= control;
        }
        if (c->pitch < 0.0f)
            pitchStep *= c->climbPitchFactor;
    } else {
        pitchStep = 0.0f;
    }
    float levelStep = levelTo != kNoInput ? AngleStep(pitch, c->pitchCorrectionRate, levelTo, seconds) : 0.0f;
    float pitchChange = levelStep + pitchStep;
    pitch += pitchChange;

    float rollChange = AngleStep(roll, c->turnRate, -(yawRate / c->turnRate) * c->bank, seconds);
    roll += rollChange;
    if (roll > 180.0f)                      // (half a turn, not a whole one; kept as the original has it)
        roll -= 180.0f;
    else if (roll < -180.0f)
        roll += 180.0f;

    position = m->Row(3);
    m->SetIdentity();
    turn.RotationY(yaw * kRadiansPerDegree);
    *m = turn * *m;
    turn.RotationX(pitch * kRadiansPerDegree);
    *m = turn * *m;
    turn.RotationZ(roll * kRadiansPerDegree);
    *m = turn * *m;
    m->SetTranslation(position);

    if (seconds > 0.001f) {
        pitchRate = pitchChange / seconds;
        rollRate = rollChange / seconds;
    } else {
        pitchRate = rollRate = 0.0f;
    }
    Body()->SetAngles(pitchRate, yawRate, rollRate);
    Body()->SetOrientation(pitch, yaw, roll);
}
