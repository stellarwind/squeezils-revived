// ~Body (SQZ.EXE 0x40bae0, vtable 0x49d484) and the CameraBody constructor / scalar deleting destructor (0x40b8a0 / 0x40b910).
#include "compat41.h"
struct BMat { float m[16]; BMat() {} ~BMat(); };                   // dtor 0x402310
struct BVec { float x, y, z; BVec() {} ~BVec(); };                 // dtor 0x401660
struct BContact { int face, second; BMat matrix; BContact() {} ~BContact(); };   // 0x48 bytes, dtor 0x4031e0
struct Body {
    virtual ~Body() {}                                             // 0x40bae0 (scalar deleting 0x40bbc0)
    BMat matrix;                                                   // 0x04
    BVec facing;                                                   // 0x44
    float facingValue;                                             // 0x50
    float angles[3];                                                // 0x54
    float unused60[3];                                             // 0x60
    float extraValue;                                              // 0x6c
    BContact contact;                                              // 0x70
    Body(void *owner);                                             // 0x41fb90
};
struct CameraBody : Body {
    int field;                                                     // 0xb8
    CameraBody(void *owner);                                       // 0x40b8a0
    virtual ~CameraBody() {}                                       // scalar deleting 0x40b910
};
CameraBody::CameraBody(void *owner) : Body(owner) { field = 0; }
CameraBody *MakeCameraBody(void *owner) { return new CameraBody(owner); }

// 0x45a940: a body that registered itself with the global registry (see Misc16/Misc2.cpp RegisterSelf)
struct HandleReg { int Lookup(int handle); void Release(int entry); };   // 0x477290 / 0x4773e0
extern HandleReg *gHandleReg;                                      // 0x4c75e0
struct HandledBody : Body {
    int padB8[2];                                                  // 0xb8
    int handle;                                                    // 0xc0
    HandledBody(void *owner) : Body(owner) { handle = 0; }
    virtual ~HandledBody();                                        // 0x45a940
};
HandledBody *MakeHandled(void *owner) { return new HandledBody(owner); }
HandledBody::~HandledBody() { gHandleReg->Release(gHandleReg->Lookup(handle)); }
