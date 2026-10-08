// SqueezilSoulInternal.h: what SqueezilSoul.cpp and SqueezilSoulMove.cpp share: calls through the body's and the soul's own vtables
// (the subclasses replace some of the soul's functions, so the soul must call itself through the table, see VirtualCall.h) and the
// matrix helpers of the original's math library (0x47dcd0-0x47e350) that the movement code uses.
#pragma once
#include <math.h>
#include "SqueezilSoul.h"
#include "SqueezilBody.h"
#include "VirtualCall.h"
#include "Original.h"

const float kRadiansPerDegree = 0.0174533f;    // the original's constant at 0x49f6ec
const float kDegreesPerRadian = 57.2958f;      // 0x49f6dc

// The seconds the last frame took
inline float FrameSeconds() { return (float)FrameMilliseconds() * 0.001f; }

inline float Dot(const Vector3f &a, const Vector3f &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vector3f ToVector(const Float3 &f) { return Vector3f(f.x, f.y, f.z); }
inline Float3 ToFloat3(const Vector3f &v) { Float3 f = { v.x, v.y, v.z }; return f; }

// A matrix's rows (right, up, forward, position) as vectors
inline Vector3f Row(const float *m, int row) { return Vector3f(m[row * 4], m[row * 4 + 1], m[row * 4 + 2]); }
inline void SetTranslation(float *m, const Vector3f &p) { m[12] = p.x; m[13] = p.y; m[14] = p.z; }      // @0047e330
// The matrix put back together from its rows, with the fourth column 0 0 0 1
inline void SetRows(float *m, const Vector3f &right, const Vector3f &up, const Vector3f &forward, const Vector3f &position)
{
    m[0]  = right.x;    m[1]  = right.y;    m[2]  = right.z;    m[3]  = 0.0f;
    m[4]  = up.x;       m[5]  = up.y;       m[6]  = up.z;       m[7]  = 0.0f;
    m[8]  = forward.x;  m[9]  = forward.y;  m[10] = forward.z;  m[11] = 0.0f;
    m[12] = position.x; m[13] = position.y; m[14] = position.z; m[15] = 1.0f;
}

// result = a * b, row-major 4x4 (result may not be a or b)                                                             @0047dea0
inline void MultiplyMatrix(float *result, const float *a, const float *b)
{
    for (int row = 0; row < 4; row++)
        for (int column = 0; column < 4; column++)
            result[row * 4 + column] = a[row * 4 + 0] * b[0 * 4 + column] + a[row * 4 + 1] * b[1 * 4 + column] +
                                       a[row * 4 + 2] * b[2 * 4 + column] + a[row * 4 + 3] * b[3 * 4 + column];
}
// m = turn * m
inline void TurnMatrix(float *m, const float *turn)
{
    float result[16];
    MultiplyMatrix(result, turn, m);
    for (int i = 0; i < 16; i++)
        m[i] = result[i];
}

// Turns about the x, y and z axes (row vectors)                                                         @0047dcd0 / 0047dd20 / 0047dd70
inline void RotationX(float *m, float radians)
{
    float c = cosf(radians), s = sinf(radians);
    m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f;  m[3] = 0.0f;
    m[4] = 0.0f; m[5] = c;    m[6] = s;     m[7] = 0.0f;
    m[8] = 0.0f; m[9] = -s;   m[10] = c;    m[11] = 0.0f;
    m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}
inline void RotationY(float *m, float radians)
{
    float c = cosf(radians), s = sinf(radians);
    m[0] = c;    m[1] = 0.0f; m[2] = -s;    m[3] = 0.0f;
    m[4] = 0.0f; m[5] = 1.0f; m[6] = 0.0f;  m[7] = 0.0f;
    m[8] = s;    m[9] = 0.0f; m[10] = c;    m[11] = 0.0f;
    m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}
inline void RotationZ(float *m, float radians)
{
    float c = cosf(radians), s = sinf(radians);
    m[0] = c;    m[1] = s;    m[2] = 0.0f;  m[3] = 0.0f;
    m[4] = -s;   m[5] = c;    m[6] = 0.0f;  m[7] = 0.0f;
    m[8] = 0.0f; m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
    m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}
// A turn of `radians` about `axis` (length 1)                                                                          @0047ddc0
inline void RotationAbout(float *m, const Vector3f &axis, float radians)
{
    float c = cosf(radians), s = sinf(radians), t = 1.0f - c;
    float x = axis.x, y = axis.y, z = axis.z;
    m[0] = x * x * t + c;      m[1] = y * x * t + z * s;  m[2] = z * x * t - y * s;  m[3] = 0.0f;
    m[4] = y * x * t - z * s;  m[5] = y * y * t + c;      m[6] = z * y * t + x * s;  m[7] = 0.0f;
    m[8] = z * x * t + y * s;  m[9] = z * y * t - x * s;  m[10] = z * z * t + c;     m[11] = 0.0f;
    m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}
// The point p moved by the matrix                                                                                      @0047e1e0
inline Vector3f TransformPoint(const Vector3f &p, const float *m)
{
    return Vector3f(m[8] * p.z + m[4] * p.y + m[0] * p.x + m[12],
                    m[9] * p.z + m[5] * p.y + m[1] * p.x + m[13],
                    m[10] * p.z + m[6] * p.y + m[2] * p.x + m[14]);
}

// The body's virtual functions (Body.h, SqueezilBody.h)
namespace BodyCall {
inline float   *Matrix(SqueezilBody *b)         { return CallVirtual<float *>(b, BODY_GET_MATRIX); }
inline Vector3f Position(SqueezilBody *b)       { return Row(Matrix(b), 3); }
inline Vector3f Direction(SqueezilBody *b)      { return ToVector(*CallVirtual<Float3 *>(b, BODY_GET_FACING)); }   // of travel
inline float    Speed(SqueezilBody *b)          { return CallVirtual<float>(b, BODY_GET_FACING_VALUE); }
inline float    Mass(SqueezilBody *b)           { return CallVirtual<float>(b, BODY_GET_EXTRA_VALUE); }
inline Contact *GetContact(SqueezilBody *b)     { return CallVirtual<Contact *>(b, BODY_GET_CONTACT); }
inline void     SetMatrix(SqueezilBody *b, const float *m)        { CallVirtual<void>(b, BODY_SET_MATRIX, m); }
inline void     SetVelocity(SqueezilBody *b, const Vector3f &direction, float speed)
{
    Float3 d = ToFloat3(direction);
    CallVirtual<void>(b, BODY_SET_FACING, (const Float3 *)&d, speed);
}
inline void     SetAngles(SqueezilBody *b, float x, float y, float z)     { CallVirtual<void>(b, BODY_SET_ANGLES, x, y, z); }
inline void     GetAngles(SqueezilBody *b, float *x, float *y, float *z)  { CallVirtual<void>(b, BODY_GET_ANGLES, x, y, z); }
inline void     SetContact(SqueezilBody *b, const Contact *c)             { CallVirtual<void>(b, BODY_SET_CONTACT, c); }
inline int      IsOnGround(SqueezilBody *b)     { return CallVirtual<int>(b, SQB_ON_GROUND); }
inline void     SetOnGround(SqueezilBody *b, int on)                      { CallVirtual<void>(b, SQB_SET_ON_GROUND, on); }
inline void     SetOrientation(SqueezilBody *b, float pitch, float yaw, float roll)     { CallVirtual<void>(b, SQB_SET_ORIENTATION, pitch, yaw, roll); }
inline void     GetOrientation(SqueezilBody *b, float *pitch, float *yaw, float *roll)  { CallVirtual<void>(b, SQB_GET_ORIENTATION, pitch, yaw, roll); }
inline void     UpdateOrientation(SqueezilBody *b)                        { CallVirtual<void>(b, SQB_UPDATE_ORIENTATION); }
inline int      GetMode(SqueezilBody *b)        { return CallVirtual<int>(b, SQB_GET_MODE); }
inline void     SetMode(SqueezilBody *b, int mode, int blend = 0)         { CallVirtual<void>(b, SQB_SET_MODE, mode, blend, 0.0f); }
inline int      ClipEnded(SqueezilBody *b)      { return CallVirtual<int>(b, SQB_CLIP_ENDED); }
inline void     SetClipSpeed(SqueezilBody *b, float speed)                { CallVirtual<void>(b, SQB_SET_CLIP_SPEED, speed); }
}

// The soul's own virtual functions, through its vtable
namespace SoulCall {
inline SqueezilBody *Body(SqueezilSoul *s)       { return CallVirtual<SqueezilBody *>(s, SQS_GET_BODY); }
inline Possessions *GetPossessions(SqueezilSoul *s)  { return CallVirtual<Possessions *>(s, SQS_GET_POSSESSIONS); }
inline Inventory *GetInventory(SqueezilSoul *s)  { return GetPossessions(s)->inventory; }          // + @00448990
inline void ResetSurface(SqueezilSoul *s)        { CallVirtual<void>(s, SQS_RESET_SURFACE); }
inline void UpdateSurface(SqueezilSoul *s, Contact c)  { CallVirtual<void>(s, SQS_UPDATE_SURFACE, c); }
inline int  CanEnter(SqueezilSoul *s, int st)    { return CallVirtual<int>(s, SQS_CAN_ENTER, st); }
inline void SetState(SqueezilSoul *s, int st, int blend)   { CallVirtual<void>(s, SQS_SET_STATE, st, blend); }
inline int  GetState(SqueezilSoul *s)            { return CallVirtual<int>(s, SQS_GET_STATE); }
inline int  GroundStateForSpeed(SqueezilSoul *s, float speed)  { return CallVirtual<int>(s, SQS_GROUND_STATE_FOR_SPEED, speed); }
inline void SpitNut(SqueezilSoul *s, float speed)  { CallVirtual<void>(s, SQS_SPIT_NUT, speed); }
inline void SprayNuts(SqueezilSoul *s)           { CallVirtual<void>(s, SQS_SPRAY_NUTS); }
inline void Steer(SqueezilSoul *s, AirControl *c, float pitchTarget, float *m, float seconds)
{
    CallVirtual<void>(s, SQS_STEER, c, pitchTarget, m, seconds);
}
inline void RotateOnSurface(SqueezilSoul *s, float *m, Vector3f *v, Contact *contact, float radians)
{
    CallVirtual<void>(s, SQS_ROTATE_ON_SURFACE, m, v, contact, radians);
}
inline int  WouldSlip(SqueezilSoul *s, float push, const float *m, const Contact *contact)
{
    return CallVirtual<int>(s, SQS_WOULD_SLIP, push, m, contact);
}
inline void MoveOnGround(SqueezilSoul *s, float seconds)  { CallVirtual<void>(s, SQS_MOVE_ON_GROUND, seconds); }
inline void StartTumble(SqueezilSoul *s)         { CallVirtual<void>(s, SQS_START_TUMBLE); }
inline void StartJump(SqueezilSoul *s, float up, float fwd, float upDistance)  { CallVirtual<void>(s, SQS_START_JUMP, up, fwd, upDistance); }
inline void AdjustNextState(SqueezilSoul *s, int *next)   { CallVirtual<void>(s, SQS_ADJUST_NEXT_STATE, next); }
inline void ReadGroundInput(SqueezilSoul *s, GroundControl *c, int *next)  { CallVirtual<void>(s, SQS_READ_GROUND_INPUT, c, next); }
inline void ReadAirInput(SqueezilSoul *s, AirControl *c, int *next)        { CallVirtual<void>(s, SQS_READ_AIR_INPUT, c, next); }
inline void GetAirMotion(SqueezilSoul *s, int st, AirControl *c)           { CallVirtual<void>(s, SQS_GET_AIR_MOTION, st, c); }
// the state handlers
inline void Handle(SqueezilSoul *s, int slot, GroundControl *c, int next)  { CallVirtual<void>(s, slot, c, next); }
inline void Handle(SqueezilSoul *s, int slot, AirControl *c, int next)     { CallVirtual<void>(s, slot, c, next); }
}

// The squeezil's position (row 3 of the body's matrix), as the sound calls take it
inline Float3 BodyPosition(SqueezilSoul *s) { return ToFloat3(BodyCall::Position(SoulCall::Body(s))); }
