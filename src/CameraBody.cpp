// CameraBody: see CameraBody.h. Reconstructed from SQZ.EXE 0x40b8a0-0x40b9f0.
#include <string.h>
#include "CameraBody.h"
#include "Scene.h"
#include "VirtualCall.h"

// @0040b8a0
CameraBody *CameraBody::Init(const float *matrix)
{
    Body::Init(matrix);
    vtbl = (void **)0x49d444;
    flags = 0;
    return this;
}

// @0040b910
CameraBody *CameraBody::DestroyAndFree(int freeFlags)
{
    Destruct();
    if (freeFlags & 1)
        delete this;
    return this;
}

// @0040b990
void CameraBody::Update()
{
    float m[16];
    memcpy(m, CallVirtual<float *>(this, BODY_GET_MATRIX), sizeof m);
    OrthonormalizeMatrix(m);
    CallVirtual<void>(this, BODY_SET_MATRIX, m);
    GetScene()->SetCameraTransform(m);
}
