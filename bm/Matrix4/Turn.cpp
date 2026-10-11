// 8 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
// Matrix4 rotation constructors (0x47dcd0 / 0x47dd20 / 0x47dd70), contiguous in the original.
#include <math.h>
#include "Matrix4.h"
Matrix4 &Matrix4::TurnX(float angle)
{
    m[0] = 1; m[1] = 0; m[2] = 0; m[3] = 0;
    m[4] = 0; m[5] = (float)cos(angle); m[6] = (float)sin(angle); m[7] = 0;
    m[8] = 0; m[9] = -(float)sin(angle); m[10] = (float)cos(angle); m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
    return *this;
}
Matrix4 &Matrix4::TurnY(float angle)
{
    m[0] = (float)cos(angle); m[1] = 0; m[2] = -(float)sin(angle); m[3] = 0;
    m[4] = 0; m[5] = 1; m[6] = 0; m[7] = 0;
    m[8] = (float)sin(angle); m[9] = 0; m[10] = (float)cos(angle); m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
    return *this;
}
Matrix4 &Matrix4::TurnZ(float angle)
{
    m[0] = (float)cos(angle); m[1] = (float)sin(angle); m[2] = 0; m[3] = 0;
    m[4] = -(float)sin(angle); m[5] = (float)cos(angle); m[6] = 0; m[7] = 0;
    m[8] = 0; m[9] = 0; m[10] = 1; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
    return *this;
}
Matrix4 &Matrix4::TurnAxis(const Vec3 &a, float angle)
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    float t = 1.0f - c;
    m[0]  = a.x*a.x*t + c;
    m[1]  = a.y*a.x*t + a.z*s;
    m[2]  = a.z*a.x*t - a.y*s;
    m[3]  = 0;
    m[4]  = a.y*a.x*t - a.z*s;
    m[5]  = a.y*a.y*t + c;
    m[6]  = a.z*a.y*t + a.x*s;
    m[7]  = 0;
    m[8]  = a.z*a.x*t + a.y*s;
    m[9]  = a.z*a.y*t - a.x*s;
    m[10] = a.z*a.z*t + c;
    m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
    return *this;
}
