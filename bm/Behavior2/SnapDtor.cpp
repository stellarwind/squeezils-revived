// Snapshot::~Snapshot (SQZ.EXE 0x402250): the three vectors (out-of-line empty dtor 0x401660) and the placement (dtor 0x4031e0).
struct SdV3 { float x, y, z; SdV3() {} ~SdV3(); };
struct SdPlacement { char pad[0x48]; SdPlacement() {} ~SdPlacement(); };
struct SdSnapshot {
    int id; float m[16];
    SdPlacement placement;
    SdV3 a, b, c;
    ~SdSnapshot();
};
SdSnapshot::~SdSnapshot() {}
