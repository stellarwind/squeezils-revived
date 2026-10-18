// SqueezilBody::UpdateOrientation (SQZ.EXE 0x4579f0): read the angles back from the matrix, wrapped into range.
#include "SqueezilBody/SqueezilBody.h"

struct XMatrix { void ToAngles(float *pitch, float *yaw, float *roll); };   // 0x47e500
extern const float kDegreesPerRadian;                    // 0x49fe1c
extern const float kFullTurn;                            // 0x49fe24

struct XBase {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void SetOrientation(float pitch, float yaw, float roll);   // slot 17
};

struct XBody2 : XBase {

    XMatrix matrix;                                      // 0x04
    void UpdateOrientation();
};

void XBody2::UpdateOrientation()
{
    float pitch, yaw, roll;
    matrix.ToAngles(&pitch, &yaw, &roll);
    pitch *= kDegreesPerRadian;
    yaw *= kDegreesPerRadian;
    roll *= kDegreesPerRadian;
    if (pitch > 270.0f)
        pitch -= kFullTurn;
    if (pitch < -90.0f)
        pitch += kFullTurn;
    if (roll > 180.0f)
        roll -= kFullTurn;
    if (roll < -180.0f)
        roll += kFullTurn;
    SetOrientation(pitch, yaw, roll);
}
