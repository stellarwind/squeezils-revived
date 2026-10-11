// The 0xb0-byte placement snapshot of a creature (copy constructor at SQZ.EXE 0x4020b0, 416 bytes).
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"

struct SnapMatrix { float v[16]; };                      // plain 64 bytes (copied with rep movs)
struct SnapMat {                                         // 64 bytes, empty out-of-line destructor 0x402310
    float a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    SnapMat() {}
    ~SnapMat();
};
struct SnapPlacement {                                   // 0x48 bytes, destructor 0x4031e0
    int prop;
    int pad;
    SnapMat at;
    SnapPlacement() {}
    SnapPlacement(const SnapPlacement &o) : prop(o.prop), pad(o.pad)
    {
        at.a0 = o.at.a0; at.a1 = o.at.a1; at.a2 = o.at.a2; at.a3 = o.at.a3;
        at.a4 = o.at.a4; at.a5 = o.at.a5; at.a6 = o.at.a6; at.a7 = o.at.a7;
        at.a8 = o.at.a8; at.a9 = o.at.a9; at.a10 = o.at.a10; at.a11 = o.at.a11;
        at.a12 = o.at.a12; at.a13 = o.at.a13; at.a14 = o.at.a14; at.a15 = o.at.a15;
    }
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
