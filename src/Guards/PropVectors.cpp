// 0x425dc0: destroy the three static prop vectors once, in reverse order.
#define V3_DTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"

struct PropVectors {
    Vector3f first;                         // 0x4c7194
    Vector3f second;                        // 0x4c71a0
    int pad;
    Vector3f third;                         // 0x4c71b0
    ~PropVectors() {}
};
extern PropVectors gPropVectors;            // 0x4c7194
extern unsigned char f_4c70e4;

void G425dc0()
{
    if (!(f_4c70e4 & 2)) {
        f_4c70e4 |= 2;
        gPropVectors.~PropVectors();
    }
}
