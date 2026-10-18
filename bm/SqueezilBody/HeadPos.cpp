// SqueezilBody virtual slot 23 (SQZ.EXE 0x457b10): the head part's position (translation row of its matrix).
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"

struct XMat {                                            // 64 bytes, empty out-of-line destructor 0x402310
    float v[12];
    Vector3f pos;                                        // 0x30
    float w;
    XMat() {}
    ~XMat();
};
struct XCycle { XMat Transform(); };                     // 0x4195b0
struct XPose { XCycle *Find(const char *name); };        // 0x41b6e0
extern const char kHeadName[];                           // 0x4bf1d4

struct XBody {
    char pad[0x22c];
    void *character;                                     // 0x22c
    Vector3f HeadPosition();                             // slot 23  0x457b10
};

Vector3f XBody::HeadPosition()
{
    return (*(XPose **)((char *)character + 0x40))->Find(kHeadName)->Transform().pos;
}
