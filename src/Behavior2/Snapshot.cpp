// The 0xb0-byte placement snapshot of a creature (copy constructor at SQZ.EXE 0x4020b0, 416 bytes).
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"

struct SnapMatrix { float v[16]; };                      // plain 64 bytes (copied with rep movs)
struct SnapMat {                                         // 64 bytes, empty out-of-line destructor 0x402310
    float a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    SnapMat() {}
    SnapMat(const SnapMat &o)
        : a0(o.a0), a1(o.a1), a2(o.a2), a3(o.a3),
          a4(o.a4), a5(o.a5), a6(o.a6), a7(o.a7),
          a8(o.a8), a9(o.a9), a10(o.a10), a11(o.a11),
          a12(o.a12), a13(o.a13), a14(o.a14), a15(o.a15) {}
    ~SnapMat();
};
struct SnapPlacement {                                   // 0x48 bytes, destructor 0x4031e0
    int prop;
    int pad;
    SnapMat at;
    SnapPlacement() {}
    SnapPlacement(const SnapPlacement &o) : prop(o.prop), pad(o.pad), at(o.at) {}
    ~SnapPlacement();
};
struct Snapshot {
    int id;                                              // 0x00
    SnapMatrix m;                                        // 0x04
    SnapPlacement placement;                             // 0x44
    Vector3f a;                                          // 0x8c
    Vector3f b;                                          // 0x98
    Vector3f c;                                          // 0xa4
    Snapshot() {}
    Snapshot(const Snapshot &o);                         // 0x4020b0
};

Snapshot::Snapshot(const Snapshot &o) : id(o.id), m(o.m), placement(o.placement), a(o.a), b(o.b), c(o.c) {}
