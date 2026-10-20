// SQZ.EXE 0x409f00: set the basis and position rows of an affine matrix.
#include "Matrix4/Matrix4.h"

Matrix4 &Matrix4::SetRows(const Vec3 &right, const Vec3 &up,
                         const Vec3 &forward, const Vec3 &position)
{
    m[0]=right.x; m[1]=right.y; m[2]=right.z; m[3]=0.0f;
    m[4]=up.x; m[5]=up.y; m[6]=up.z; m[7]=0.0f;
    m[8]=forward.x; m[9]=forward.y; m[10]=forward.z; m[11]=0.0f;
    m[12]=position.x; m[13]=position.y;
    float z = position.z;
    m[15]=1.0f;
    m[14]=z;
    return *this;
}
