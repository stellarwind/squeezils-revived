// FrameProp virtual slot 18 (SQZ.EXE 0x427e90): the matrix of the body's named cycle.
#include "FrameProp/FrameProp.h"

struct XMat {                                            // 64 bytes, empty out-of-line destructor 0x402310
    float v[16];
    XMat() {}
    ~XMat();
};
struct XCycle { XMat Transform(); };                     // 0x4195b0
struct XPose { XCycle *Find(const char *name); };        // 0x41b6e0
extern const char kPoseName[];                           // 0x4b9a7c

struct XFrame {                                          // FrameProp seen from the matrix accessor
    char pad[0x94];
    void *body;                                          // 0x94
    XMat CurrentPose();                                  // slot 18  0x427e90
};

XMat XFrame::CurrentPose()
{
    return (*(XPose **)((char *)body + 0x40))->Find(kPoseName)->Transform();
}
