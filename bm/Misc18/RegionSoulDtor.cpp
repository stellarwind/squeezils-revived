// SqueezilRegionSoul scalar deleting destructor (SQZ.EXE 0x4369c0): two 12-byte members with out-of-line dtors, base dtor at 0x44f600.
#include "compat41.h"
struct Part { int v[3]; Part() {} ~Part(); };
struct SoulBase { char pad[0x94]; virtual ~SoulBase(); };      // ~SoulBase = 0x44f600
struct RS : SoulBase {
    Part a;      // +0x98
    Part b;      // +0xa4
    char rest[0x20];
    RS() {}
    virtual ~RS() {}
};
RS *ForceRS() { return new RS; }
