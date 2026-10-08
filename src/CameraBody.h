// CameraBody: the body the camera is. Reconstructed from SQZ.EXE 0x40b8a0-0x40b9f0.
//
// Nothing is drawn for it; each update squares up its matrix and hands the matrix to the scene as the camera's view.
#pragma once
#include "Body.h"

struct CameraBody : Body {
    int flags;      // 0xb8 the simulation reads this word on every actor (starts 0)

    CameraBody *Init(const float *matrix);      // @0040b8a0
    CameraBody *DestroyAndFree(int flags);      // @0040b910 slot 0 (bit 0 of flags: free the memory)
    void Update();                              // @0040b990 slot 2
};
static_assert(sizeof(CameraBody) == 0xbc, "CameraBody layout");
